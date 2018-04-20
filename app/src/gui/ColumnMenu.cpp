#include "ColumnMenu.h"

#include <QTableView>
#include <QEvent>

ColumnActionWrapper::ColumnActionWrapper(int col, QObject *parent) :
	QAction(parent), m_column(col)
{
	setCheckable(true);
	connect(this, SIGNAL(triggered(bool)), this, SLOT(triggeredSlot(bool)));
}

void ColumnActionWrapper::triggeredSlot(bool checked) {
	if (checked) {
		emit showColumn(column());
	} else {
		emit hideColumn(column());
	}
}

ColumnMenu::ColumnMenu(QWidget *parent) :
	QMenu(parent), view(NULL), model(NULL) {
}

void ColumnMenu::setView(QTableView *t) {
	clearMenu();
	if (view) disconnect(view);
	if (model) disconnect(model);
	view = t;
	model = t ? t->model() : NULL;
	if (view) connect(view, SIGNAL(destroyed()), this, SLOT(viewDestroyed()));
	if (model) {
		connect(model, SIGNAL(destroyed()), this, SLOT(modelDestroyed()));
		connect(model, SIGNAL(modelReset()), this, SLOT(rebuildMenu()));
		connect(model, SIGNAL(columnsInserted(const QModelIndex &, int, int)),
		        this, SLOT(columnsInserted(const QModelIndex &, int, int)));
		connect(model, SIGNAL(columnsRemoved(const QModelIndex &, int, int)),
		        this, SLOT(columnsRemoved(const QModelIndex &, int, int)));
		connect(model, SIGNAL(headerDataChanged(Qt::Orientation, int, int)),
		        this, SLOT(headerDataChanged(Qt::Orientation, int, int)));
		connect(model, SIGNAL(columnsMoved(const QModelIndex &, int, int, const QModelIndex &, int)),
		        this, SLOT(columnsMoved(const QModelIndex &, int, int, const QModelIndex &, int)));
	}
	rebuildMenu();
}

void ColumnMenu::rebuildMenu() {
	clearMenu();
	if (model && model->columnCount() > 0) {
		columnsInserted(QModelIndex(), 0, model->columnCount() - 1);
	}
}

void ColumnMenu::viewDestroyed() {
	view = NULL;
	clearMenu();
}

void ColumnMenu::modelDestroyed() {
	model = NULL;
	clearMenu();
}

void ColumnMenu::columnsInserted(const QModelIndex &, int first, int last) {
	QList<ColumnActionWrapper *>::iterator pos = columnMenuActions.end();
	if (first < columnMenuActions.count()) {
		pos = columnMenuActions.begin() + first;
	}
	if (view && model) {
		for (int i = first; i <= last; ++i) {
			ColumnActionWrapper *act = new ColumnActionWrapper(i, this);
			act->setChecked(!view->isColumnHidden(i));
			act->setText(model->headerData(i, Qt::Horizontal).toString());
			connect(act, SIGNAL(showColumn(int)), view, SLOT(showColumn(int)));
			connect(act, SIGNAL(hideColumn(int)), view, SLOT(hideColumn(int)));
			if (pos == columnMenuActions.end()) {
				addAction(act);
				columnMenuActions.append(act);
				pos = columnMenuActions.end();
			} else {
				insertAction(*pos, act);
				pos = columnMenuActions.insert(pos, act);
				++pos;
			}
		}
		for (int i = last + 1; i < columnMenuActions.count(); ++i) {
			columnMenuActions[i]->setColumn(i);
		}
	}
}
void ColumnMenu::columnsRemoved(const QModelIndex &, int first, int last) {
	if (first < columnMenuActions.count() && first <= last) {
		QList<ColumnActionWrapper *>::iterator start =
			columnMenuActions.begin() + first;
		QList<ColumnActionWrapper *>::iterator end = columnMenuActions.end();
		if (last < columnMenuActions.count() - 1) {
			end = columnMenuActions.begin() + last + 1;
		}
		for (QList<ColumnActionWrapper *>::iterator it = start; it != end; ++it) {
			removeAction(*it);
			(*it)->deleteLater();
		}
		columnMenuActions.erase(start, end);
	}
}

void ColumnMenu::columnsMoved(const QModelIndex &, int start, int end,
                              const QModelIndex &, int column) {
	if (column == start) return;
	if (end >= columnMenuActions.count()) {
		end = columnMenuActions.count() - 1;
	}
	if (end < start) return;
	if (end - start + column >= columnMenuActions.count()) {
		end = columnMenuActions.count() + start - column - 1;
	}
	if (end < start) return;

	int ofs, count, mv;
	if (start > column) {
		ofs = column;
		count = 1 + end - column;
		mv = count - (start - column);
	} else {
		ofs = start;
		count = 1 + (end - start + column) - start;
		mv = column - start;
	}
	int i = 0;
	int cnt = 0;
	while (cnt < count) {
		int di = (i + mv) % count;
		ColumnActionWrapper *t = columnMenuActions[ofs + di];
		columnMenuActions[ofs + di] = columnMenuActions[ofs + i];
		while (di != i) {
			di = (di + mv) % count;
			ColumnActionWrapper *t2 = columnMenuActions[ofs + di];
			columnMenuActions[ofs + di] = t;
			t = t2;
			++cnt;
		}
		++i;
		++cnt;
	}
	int d = count - mv;
	for (i = ofs; i < ofs + count; ++i) {
		if (d > mv) {
			if (columnMenuActions[i]->column() > i) {
				removeAction(columnMenuActions[i + d]);
				insertAction(columnMenuActions[i], columnMenuActions[i + d]);
			}
		} else {
			if (columnMenuActions[i]->column() < i) {
				removeAction(columnMenuActions[i]);
				insertAction(columnMenuActions[i - mv], columnMenuActions[i]);
			}
		}
		columnMenuActions[i]->setColumn(i);
	}
}

void ColumnMenu::headerDataChanged(Qt::Orientation orientation, int first, int last) {
	if (orientation == Qt::Horizontal && model) {
		for (int i = first; i <= last && i < columnMenuActions.count(); ++i) {
			columnMenuActions[i]->setText(model->headerData(i, Qt::Horizontal).toString());
		}
	}
}

void ColumnMenu::changeEvent(QEvent *event) {
	if (event) {
		switch (event->type()) {
		case QEvent::LanguageChange:
			headerDataChanged(Qt::Horizontal, 0, columnMenuActions.count());
			break;
		default:
			break;
		}
	}
	QMenu::changeEvent(event);
}

void ColumnMenu::clearMenu() {
	if (!columnMenuActions.isEmpty()) {
		columnsRemoved(QModelIndex(), 0, columnMenuActions.count() - 1);
	}
}
