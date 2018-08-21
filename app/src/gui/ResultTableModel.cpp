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
	dataLanguage(LANG_EN),
	monsterMode(MODE_ENRAGED_WEAK_SPOT)
{
}

ResultTableModel::~ResultTableModel() {
	clearData();
}

bool ResultTableModel::isNumberColumn(Column c) {
	switch (c) {
	case COLUMN_TOTAL_DPS:
	case COLUMN_RAW_DPS:
	case COLUMN_ELEMENT_DPS:
	case COLUMN_STATUS_DPS:
	case COLUMN_FIXED_DPS:
	case COLUMN_KILL_TIME:
	case COLUMN_BOUNCE_RATE:
	case COLUMN_CRIT_RATE:
	case COLUMN_POISON_PROC_RATE:
	case COLUMN_PARALYSIS_PROC_RATE:
	case COLUMN_SLEEP_PROC_RATE:
	case COLUMN_STUN_PROC_RATE:
	case COLUMN_BLAST_PROC_RATE:
	case COLUMN_EXHAUST_PROC_RATE:
	case COLUMN_MOUNT_PROC_RATE:
	case COLUMN_SHARPNESS_USE:
	case COLUMN_DMG_CUT_PIERCING:
	case COLUMN_DMG_IMPACT:
	case COLUMN_DMG_FIXED:
	case COLUMN_DMG_ELEMENT:
	case COLUMN_DMG_STATUS:
	case COLUMN_DMG_STUN:
	case COLUMN_DMG_EXHAUST:
	case COLUMN_DMG_MOUNT:
		return true;
	default:
		return false;
	}
}

bool ResultTableModel::isGreaterIsBetterColumn(Column c) {
	switch (c) {
	case COLUMN_TOTAL_DPS:
	case COLUMN_RAW_DPS:
	case COLUMN_ELEMENT_DPS:
	case COLUMN_STATUS_DPS:
	case COLUMN_FIXED_DPS:
	case COLUMN_CRIT_RATE:
	case COLUMN_POISON_PROC_RATE:
	case COLUMN_PARALYSIS_PROC_RATE:
	case COLUMN_SLEEP_PROC_RATE:
	case COLUMN_STUN_PROC_RATE:
	case COLUMN_BLAST_PROC_RATE:
	case COLUMN_EXHAUST_PROC_RATE:
	case COLUMN_MOUNT_PROC_RATE:
	case COLUMN_DMG_CUT_PIERCING:
	case COLUMN_DMG_IMPACT:
	case COLUMN_DMG_FIXED:
	case COLUMN_DMG_ELEMENT:
	case COLUMN_DMG_STATUS:
	case COLUMN_DMG_STUN:
	case COLUMN_DMG_EXHAUST:
	case COLUMN_DMG_MOUNT:
		return true;
	default:
		return false;
	}
}

bool ResultTableModel::isDefaultVisibleColumn(Column c) {
	switch (c) {
	case COLUMN_TOTAL_DPS:
	case COLUMN_RAW_DPS:
	case COLUMN_ELEMENT_DPS:
	case COLUMN_STATUS_DPS:
	case COLUMN_FIXED_DPS:
	case COLUMN_WEAPON_NAME:
		return true;
	default:
		return c >= COLUMN_COUNT;
	}
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
	case COLUMN_CRIT_RATE:
		return tr("Critical");
	case COLUMN_KILL_TIME:
		return tr("Kill time");
	case COLUMN_POISON_PROC_RATE:
		return tr("Poison");
	case COLUMN_PARALYSIS_PROC_RATE:
		return tr("Paralysis");
	case COLUMN_SLEEP_PROC_RATE:
		return tr("Sleep");
	case COLUMN_STUN_PROC_RATE:
		return tr("Stun");
	case COLUMN_BLAST_PROC_RATE:
		return tr("Blast");
	case COLUMN_EXHAUST_PROC_RATE:
		return tr("Exhaust");
	case COLUMN_MOUNT_PROC_RATE:
		return tr("Mount");
	case COLUMN_SHARPNESS_USE:
		return tr("Sharpness use");
	case COLUMN_DMG_CUT_PIERCING:
		return tr("Base cut");
	case COLUMN_DMG_IMPACT:
		return tr("Base impact");
	case COLUMN_DMG_FIXED:
		return tr("Base fixed");
	case COLUMN_DMG_ELEMENT:
		return tr("Base element");
	case COLUMN_DMG_STATUS:
		return tr("Base status");
	case COLUMN_DMG_STUN:
		return tr("Base stun");
	case COLUMN_DMG_EXHAUST:
		return tr("Base exhaust");
	case COLUMN_DMG_MOUNT:
		return tr("Base mount");
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
	return COLUMN_COUNT + itemColumns;
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
	if (index.row() < 0 || index.row() >= resultData.count()) return QVariant();

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
			case COLUMN_KILL_TIME:
				return 1.0 / dps.killFrequency;
			case COLUMN_BOUNCE_RATE:
				return dps.bounceRate;
			case COLUMN_CRIT_RATE:
				return dps.critRate;
			case COLUMN_POISON_PROC_RATE:
				return dps.statusProcRate[STATUS_POISON];
			case COLUMN_PARALYSIS_PROC_RATE:
				return dps.statusProcRate[STATUS_PARALYSIS];
			case COLUMN_SLEEP_PROC_RATE:
				return dps.statusProcRate[STATUS_SLEEP];
			case COLUMN_STUN_PROC_RATE:
				return dps.statusProcRate[STATUS_STUN];
			case COLUMN_BLAST_PROC_RATE:
				return dps.statusProcRate[STATUS_BLAST];
			case COLUMN_EXHAUST_PROC_RATE:
				return dps.statusProcRate[STATUS_EXHAUST];
			case COLUMN_MOUNT_PROC_RATE:
				return dps.statusProcRate[STATUS_MOUNT];
			case COLUMN_SHARPNESS_USE:
				return bwd->damage.sharpenPeriod * bwd->damage.sharpnessUse[0];
			case COLUMN_DMG_CUT_PIERCING:
				return bwd->damage.data[monsterMode]->cut +
					bwd->damage.data[monsterMode]->piercing;
			case COLUMN_DMG_IMPACT:
				return bwd->damage.data[monsterMode]->impact;
			case COLUMN_DMG_FIXED:
				return bwd->damage.data[monsterMode]->fixed;
			case COLUMN_DMG_ELEMENT:
				{
					double total = 0.0;
					for (int i = 0; i < ELEMENT_COUNT; ++i) {
						total += bwd->damage.data[monsterMode]->elements[i];
					}
					return total;
				}
			case COLUMN_DMG_STATUS:
				{
					double total = 0.0;
					for (int i = 0; i < STATUS_COUNT; ++i) {
						if (i == STATUS_STUN) continue;
						if (i == STATUS_EXHAUST) continue;
						if (i == STATUS_MOUNT) continue;
						total += bwd->damage.data[monsterMode]->statuses[i];
					}
					return total;
				}
			case COLUMN_DMG_STUN:
				return bwd->damage.data[monsterMode]->statuses[STATUS_STUN];
			case COLUMN_DMG_EXHAUST:
				return bwd->damage.data[monsterMode]->statuses[STATUS_EXHAUST];
			case COLUMN_DMG_MOUNT:
				return bwd->damage.data[monsterMode]->statuses[STATUS_MOUNT];
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
		if (index.column() == COLUMN_ELEMENT_DPS ||
		    index.column() == COLUMN_DMG_ELEMENT) {
			return GuiElements::elementBrush(bwd->damage.data[monsterMode]->elements);
		} else if (index.column() == COLUMN_STATUS_DPS ||
		           index.column() == COLUMN_DMG_STATUS) {
			return GuiElements::statusBrush(bwd->damage.data[monsterMode]->statuses);
		} else {
			return QVariant();
		}
	} else if (role == Qt::TextAlignmentRole) {
		if (isNumberColumn((Column)index.column())) {
			return QVariant(Qt::AlignRight | Qt::AlignVCenter);
		} else {
			return QVariant();
		}
	} else if (role == Qt::ToolTipRole) {
		if (index.column() == COLUMN_ELEMENT_DPS ||
		    index.column() == COLUMN_DMG_ELEMENT) {
			return GuiElements::elementToolTip(bwd->damage.data[monsterMode]->elements);
		} else if (index.column() == COLUMN_STATUS_DPS ||
		           index.column() == COLUMN_DMG_STATUS) {
			return GuiElements::statusToolTip(bwd->damage.data[monsterMode]->statuses);
		} else {
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
			rows[index.row()] = -1;
			columns[index.column()] = -1;
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
	foreach(int c, mimeColumnOrder) {
		QMap<int, int>::iterator it = columns.find(c);
		if (it != columns.end()) *it = idx++;
	}
	for (QMap<int, int>::iterator it = columns.begin();
	     it != columns.end(); ++it) {
		if (*it == -1) *it = idx++;
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
		mimeData->setText(data(indexes[0]).toString());
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

	if (isGreaterIsBetterColumn((Column)column)) {
		if (order == Qt::AscendingOrder) {
			std::stable_sort(rows.begin(), rows.end(), greaterRowWithKey);
		} else {
			std::stable_sort(rows.begin(), rows.end(), lessRowWithKey);
		}
	} else {
		if (order == Qt::AscendingOrder) {
			std::stable_sort(rows.begin(), rows.end(), lessRowWithKey);
		} else {
			std::stable_sort(rows.begin(), rows.end(), greaterRowWithKey);
		}
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

struct MapDummy { };
QVector<BuildWithDps *> ResultTableModel::resultDataList(const QModelIndexList &indexes) const {
	QMap<int, MapDummy> rows;
	QVector<BuildWithDps *> ret;

	foreach(const QModelIndex &index, indexes) {
		if (index.isValid() && index.row() >= 0 && index.row() < rowCount()) {
			rows.insert(index.row(), MapDummy());
		}
	}
	ret.reserve(rows.count());
	for (QMap<int, MapDummy>::iterator it = rows.begin();
	     it != rows.end(); ++it) {
		ret.append(resultData[it.key()]);
	}
	return ret;
}

void ResultTableModel::setResultData(const QVector<BuildWithDps *> &d) {
	clear();
	int item_columns = 0;
	foreach(BuildWithDps *bwd, d) {
		if (item_columns < bwd->build->usedItems.count()) {
			item_columns = bwd->build->usedItems.count();
		}
	}
	if (item_columns > itemColumns) {
		beginInsertColumns(QModelIndex(),
		                   COLUMN_COUNT + itemColumns,
		                   COLUMN_COUNT + item_columns - 1);
		itemColumns = item_columns;
		endInsertColumns();
	} else if (item_columns < itemColumns) {
		beginRemoveColumns(QModelIndex(),
		                   COLUMN_COUNT + item_columns,
		                   COLUMN_COUNT + itemColumns - 1);
		itemColumns = item_columns;
		endRemoveColumns();
	}
	if (d.count() > 0) {
		beginInsertRows(QModelIndex(), 0, d.count() - 1);
		resultData = d;
		endInsertRows();
	}
}

void ResultTableModel::clear() {
	if (rowCount() > 0) {
		beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
		clearData();
		endRemoveRows();
	} else {
		clearData();
	}
}

void ResultTableModel::setMimeColumnOrder(QList<int> order) {
	mimeColumnOrder = order;
}

void ResultTableModel::setDataLanguage(Language lang) {
	dataLanguage = lang;
	emit dataChanged(index(0, COLUMN_WEAPON_NAME),
	                 index(rowCount() - 1, columnCount() - 1));
}

void ResultTableModel::setMonsterMode(MonsterMode mode) {
	monsterMode = mode;
	emit dataChanged(index(0, COLUMN_DMG_CUT_PIERCING),
	                 index(rowCount() - 1, COLUMN_DMG_MOUNT));
}

void ResultTableModel::clearData() {
	foreach(BuildWithDps *bwd, resultData) delete bwd;
	resultData.clear();
}
