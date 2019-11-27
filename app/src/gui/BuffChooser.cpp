#include "BuffChooser.h"
#include "ui_BuffChooser.h"

#include "BuffListModel.h"
#include "BuffGroupListModel.h"

BuffChooser::BuffChooser(QWidget *parent) :
	QWidget(parent), ui(new Ui::BuffChooser),
	m_buffListModel(new BuffListModel(this)),
	m_buffGroupListModel(new BuffGroupListModel(this))
{
	ui->setupUi(this);
	ui->buffCb->setModel(m_buffGroupListModel);
	ui->buffListView->setModel(m_buffListModel);
	connect(ui->buffListView->selectionModel(),
	        SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
	        this, SLOT(updateBuffGroupFromListSelection()));
	connect(ui->buffCb, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(buffGroupChanged(int)));
	connect(ui->addBuff, SIGNAL(clicked()), this, SLOT(addBuff()));
	connect(ui->removeBuff, SIGNAL(clicked()), this, SLOT(removeBuff()));
}

void BuffChooser::setDataLanguage(Language lang)
{
	m_buffListModel->setDataLanguage(lang);
	m_buffGroupListModel->setDataLanguage(lang);
}

void BuffChooser::changeEvent(QEvent *event) {
	if (event) {
		switch (event->type()) {
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;
		default:
			break;
		}
	}
	QWidget::changeEvent(event);
}

void BuffChooser::selectBuffGroupFromList(const QModelIndex &index) {
	if (!index.isValid()) return;
	const BuffGroup *group = NULL;
	{
		const BuffListModel *model =
			qobject_cast<const BuffListModel *>(index.model());
		if (model) group = model->buffGroup(index);
	}
	if (group) {
		const BuffGroupListModel *model =
			qobject_cast<const BuffGroupListModel *>(ui->buffCb->model());
		if (model) {
			QModelIndex idx = model->buffGroupIndex(group);
			if (idx.isValid()) {
				ui->buffCb->setCurrentIndex(idx.row());
			}
		}
	}
}

void BuffChooser::buffGroupChanged(int new_idx) {
	const BuffGroup *group = NULL;
	if (new_idx >= 0) {
		const BuffGroupListModel *model =
			qobject_cast<const BuffGroupListModel *>(ui->buffCb->model());
		if (model) group = model->buffGroup(model->index(new_idx));
	}
	if (group) {
		QModelIndex idx = m_buffListModel->buffGroupIndex(group);
		if (idx.isValid()) {
			ui->buffListView->selectionModel()->
				select(idx, QItemSelectionModel::ClearAndSelect);
		} else {
			ui->buffListView->selectionModel()->clearSelection();
		}
	}
}

void BuffChooser::addBuff() {
	const BuffGroup *bg = getBuffGroup();
	if (bg) {
		m_buffListModel->addBuff(bg, 1);
		QModelIndex idx = m_buffListModel->buffGroupIndex(bg);
		if (idx.isValid()) {
			ui->buffListView->setCurrentIndex(idx);
			ui->buffListView->scrollTo(idx);
		}
	}
}

void BuffChooser::removeBuff() {
	const BuffGroup *bg = getBuffGroup();
	if (bg) {
		m_buffListModel->removeBuff(bg, 1);
		QModelIndex idx = m_buffListModel->buffGroupIndex(bg);
		if (idx.isValid()) {
			ui->buffListView->setCurrentIndex(idx);
			ui->buffListView->scrollTo(idx);
		}
	}
}

void BuffChooser::updateBuffGroupFromListSelection() {
	QModelIndexList sel = ui->buffListView->selectionModel()->selectedIndexes();
	if (sel.count() == 1) selectBuffGroupFromList(sel[0]);
}

const BuffGroup *BuffChooser::getBuffGroup() const {
	int idx = ui->buffCb->currentIndex();

	if (idx >= 0) {
		const BuffGroupListModel *model =
			qobject_cast<const BuffGroupListModel *>(ui->buffCb->model());
		if (model) return model->buffGroup(model->index(idx));
	}
	return NULL;
}
