#include "ResultTableModel.h"

#include <QMimeData>
#include <algorithm>
#include "GuiElements.h"
#include "../BuildWithDps.h"
#include "../Build.h"
#include "../DamageData.h"
#include "../Weapon.h"
#include "../Item.h"

ResultTableModel::ResultTableModel(QObject *parent) :
	QAbstractTableModel(parent), itemColumns(0),
	dataLanguage(NamedObject::LANG_EN)
{
}

ResultTableModel::~ResultTableModel() {
	clearData();
}

QString ResultTableModel::columnName(Column c) {
	switch (c) {
	case COLUMN_TOTAL_DPS:
		return tr("Total");
	case COLUMN_RAW_DPS:
		return tr("Raw");
	case COLUMN_ELEMENT_DPS:
		return tr("Element");
	case COLUMN_STATUS_DPS:
		return tr("Status");
	case COLUMN_FIXED_DPS:
		return tr("Fixed");
	case COLUMN_BOUNCE_RATE:
		return tr("Bounce");
	case COLUMN_SHARPNESS_USE:
		return tr("Sharpness use");
	case COLUMN_WEAPON_NAME:
		return tr("Weapon");
	case COLUMN_COUNT:
		return QString();
	}
	return QString();
}

int ResultTableModel::rowCount(const QModelIndex &parent) const {
	if (parent.isValid()) return 0;
	return resultData.count();
}

int ResultTableModel::columnCount(const QModelIndex &parent) const {
	if (parent.isValid()) return 0;
	return 7 + itemColumns;
}

QVariant ResultTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Horizontal) {
		if (role == Qt::DisplayRole) {
			if (section >= COLUMN_COUNT)
				return tr("Item %1").arg(1 + section - COLUMN_COUNT);
			if (section >= 0) {
				return columnName((Column)section);
			} else {
				return QVariant();
			}
		} else {
			return QVariant();
		}
	} else {
		if (role == Qt::DisplayRole) {
			return section + 1;
		} else if (role == Qt::TextAlignmentRole) {
			return QVariant(Qt::AlignRight | Qt::AlignVCenter);
		} else {
			return QVariant();
		}
	}
}

QVariant ResultTableModel::data(const QModelIndex &index, int role) const {
	if (!index.isValid()) return QVariant();
	if (index.row() < 0 && index.row() >= resultData.count()) return QVariant();

	BuildWithDps *bwd = resultData[index.row()];
	const Dps &dps = bwd->dps;
	if (role == Qt::DisplayRole) {
		if (index.column() >= COLUMN_COUNT + bwd->build->usedItems.count()) {
			return QVariant();
		} else if (index.column() >= COLUMN_COUNT) {
			return bwd->build->usedItems[index.column() - COLUMN_COUNT]->
				getName(dataLanguage);
		} else if (index.column() >= 0) {
			switch ((Column)index.column()) {
			case COLUMN_TOTAL_DPS:
				return dps.raw + dps.totalElements + dps.totalStatuses +
					dps.fixed;
			case COLUMN_RAW_DPS:
				return dps.raw;
			case COLUMN_ELEMENT_DPS:
				return dps.totalElements;
			case COLUMN_STATUS_DPS:
				return dps.totalStatuses;
			case COLUMN_FIXED_DPS:
				return dps.fixed;
			case COLUMN_BOUNCE_RATE:
				return dps.bounceRate;
			case COLUMN_SHARPNESS_USE:
				return bwd->damage.sharpenPeriod * bwd->damage.sharpnessUse[0];
			case COLUMN_WEAPON_NAME:
				return bwd->build->weapon->getName(dataLanguage);
			case COLUMN_COUNT:
				return QVariant();
			}
			return QVariant();
		} else {
			return QVariant();
		}
	} else if (role == Qt::BackgroundRole) {
		if (index.column() == COLUMN_ELEMENT_DPS) {
			return GuiElements::elementBrush(bwd->damage.data[MODE_ENRAGED_WEAK_SPOT]->elements);
		} else if (index.column() == COLUMN_STATUS_DPS) {
			return GuiElements::statusBrush(bwd->damage.data[MODE_ENRAGED_WEAK_SPOT]->statuses);
		} else {
			return QVariant();
		}
	} else if (role == Qt::TextAlignmentRole) {
		switch ((Column)index.column()) {
		case COLUMN_TOTAL_DPS:
		case COLUMN_RAW_DPS:
		case COLUMN_ELEMENT_DPS:
		case COLUMN_STATUS_DPS:
		case COLUMN_FIXED_DPS:
		case COLUMN_BOUNCE_RATE:
		case COLUMN_SHARPNESS_USE:
			return QVariant(Qt::AlignRight | Qt::AlignVCenter);
		default:
			return QVariant();
		}
	} else {
		return QVariant();
	}
}

QVector<QVector<QVariant> > ResultTableModel::getDataTable(const QModelIndexList &indexes) const {
	QMap<int, int> rows;
	QMap<int, int> columns;
	QVector<QVector<QVariant> > out_table;

	foreach(const QModelIndex &index, indexes) {
		if (index.isValid() &&
		    index.row() >= 0 && index.row() < rowCount() &&
		    index.column() >= 0 && index.column() < columnCount()) {
			rows[index.row()] = 1;
			columns[index.column()] = 1;
		}
	}

	int idx = 0;
	out_table.reserve(rows.count());
	for (QMap<int, int>::iterator it = rows.begin();
	     it != rows.end(); ++it) {
		*it = idx++;
		QVector<QVariant> r;
		r.resize(columns.count());
		out_table.append(r);
	}
	idx = 0;
	for (QMap<int, int>::iterator it = columns.begin();
	     it != columns.end(); ++it) {
		*it = idx++;
	}

	foreach(const QModelIndex &index, indexes) {
		if (index.isValid()) {
			QMap<int, int>::const_iterator itr = rows.find(index.row());
			QMap<int, int>::const_iterator itc = columns.find(index.column());
			if (itr != rows.end() && itc != columns.end()) {
				out_table[*itr][*itc] = data(index);
			}
		}
	}

	return out_table;
}

QMimeData *ResultTableModel::mimeData(const QModelIndexList &indexes) const {
	QMimeData *mimeData = NULL;

	if (indexes.count() > 0) {
		if (!mimeData) mimeData = new QMimeData();
		QVector<QVector<QVariant> > table = getDataTable(indexes);

		QString html("<html><style>br{mso-data-placement:same-cell;}</style><table>");
		for (int r = 0; r < table.count(); ++r) {
			html.append("<tr>");
			for (int c = 0; c < table[r].count(); ++c) {
				QString v = table[r][c].toString();
				v.replace("&", "&amp;");
				v.replace("<", "&lt;");
				v.replace(">", "&gt;");
				v.replace("\n", "<br/>");
				html.append("<td>");
				html.append(v);
				html.append("</td>");
			}
			html.append("</tr>");
		}
		html.append("</table></html>");
		mimeData->setHtml(html);

		QString csv;
		for (int r = 0; r < table.count(); ++r) {
			for (int c = 0; c < table[r].count(); ++c) {
				if (c > 0) csv.append(",");
				QString v = table[r][c].toString();
				if (v.contains(',') || v.contains('\n') || v.contains('"')) {
					v = v.replace("\"", "\"\"");
					csv.append('"');
					csv.append(v);
					csv.append('"');
				} else {
					csv.append(v);
				}
			}
			csv.append("\r\n");
		}
		mimeData->setData("text/csv", csv.toUtf8());
	}

	if (indexes.count() == 1) {
		if (!mimeData) mimeData = new QMimeData();
		mimeData->setText(data(indexes[0]).toString().toUtf8());
	}
	return mimeData;
}

Qt::ItemFlags ResultTableModel::flags(const QModelIndex &index) const {
	if (index.isValid() &&
	    index.row() >= 0 && index.row() < rowCount() &&
	    index.column() >= 0 && index.column() < columnCount()) {
		return Qt::ItemIsSelectable | Qt::ItemIsEnabled |
			Qt::ItemIsDragEnabled;
	} else {
		return 0;
	}
}

struct RowWithKey {
	RowWithKey() :
		key(), bwd(NULL), row_idx(0) { }
	RowWithKey(QVariant k, BuildWithDps *d, int i) :
		key(k), bwd(d), row_idx(i) { }
	QVariant key;
	BuildWithDps *bwd;
	int row_idx;
};
bool lessRowWithKey(const RowWithKey &a, const RowWithKey &b) {
	if (!a.key.isValid() || !b.key.isValid()) {
		return b.key.isValid();
	}
	if (a.key.type() == QVariant::Double && b.key.type() == QVariant::Double) {
		return a.key.toDouble() < b.key.toDouble();
	}
	return a.key.toString() < b.key.toString();
}
bool greaterRowWithKey(const RowWithKey &a, const RowWithKey &b) {
	return lessRowWithKey(b, a);
}

void ResultTableModel::sort(int column, Qt::SortOrder order) {
	QVector<RowWithKey> rows;

	rows.reserve(resultData.count());
	for (int i = 0; i < resultData.count(); ++i) {
		rows.append(RowWithKey(data(index(i, column)), resultData[i], i));
	}

	switch (column) {
	case COLUMN_TOTAL_DPS:
	case COLUMN_RAW_DPS:
	case COLUMN_ELEMENT_DPS:
	case COLUMN_STATUS_DPS:
	case COLUMN_FIXED_DPS:
		if (order == Qt::AscendingOrder) {
			std::stable_sort(rows.begin(), rows.end(), greaterRowWithKey);
		} else {
			std::stable_sort(rows.begin(), rows.end(), lessRowWithKey);
		}
		break;
	default:
		if (order == Qt::AscendingOrder) {
			std::stable_sort(rows.begin(), rows.end(), lessRowWithKey);
		} else {
			std::stable_sort(rows.begin(), rows.end(), greaterRowWithKey);
		}
		break;
	}

	emit layoutAboutToBeChanged();
	if (!persistentIndexList().isEmpty()) {
		QModelIndexList old_idx = persistentIndexList();
		QVector<int> reverse_map;
		reverse_map.resize(resultData.count());
		for (int i = 0; i < resultData.count(); ++i) {
			reverse_map[rows[i].row_idx] = i;
		}
		QModelIndexList new_idx;
		foreach(const QModelIndex &idx, old_idx) {
			new_idx.append(index(reverse_map[idx.row()],
			                     idx.column(), idx.parent()));
		}
		changePersistentIndexList(old_idx, new_idx);
	}
	for (int i = 0; i < resultData.count(); ++i) {
		resultData[i] = rows[i].bwd;
	}
	emit layoutChanged();
}

void ResultTableModel::setDataLanguage(NamedObject::Language lang) {
	dataLanguage = lang;
	emit dataChanged(index(0, 6), index(rowCount() - 1, columnCount() - 1));
}

void ResultTableModel::setResultData(const QVector<BuildWithDps *> &d) {
	beginResetModel();
	clearData();
	resultData = d;
	itemColumns = 0;
	foreach(BuildWithDps *bwd, resultData) {
		if (itemColumns < bwd->build->usedItems.count()) {
			itemColumns = bwd->build->usedItems.count();
		}
	}
	endResetModel();
}

void ResultTableModel::clear() {
	beginResetModel();
	clearData();
	endResetModel();
}

void ResultTableModel::clearData() {
	foreach(BuildWithDps *bwd, resultData) delete bwd;
	resultData.clear();
	itemColumns = 0;
}
