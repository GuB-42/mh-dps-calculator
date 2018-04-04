#include "BuffListModel.h"
#include "../Build.h"
#include "../BuffGroup.h"

BuffListModel::BuffListModel(QObject *parent) :
	QAbstractListModel(parent), dataLanguage(NamedObject::LANG_EN)
{
}

int BuffListModel::rowCount(const QModelIndex &parent) const {
	if (parent.isValid()) return 0;
	return listData.count();
}

QVariant BuffListModel::data(const QModelIndex &index, int role) const {
	if (!index.isValid()) return QVariant();
	if (index.row() < 0 && index.row() >= listData.count()) return QVariant();

	if (role == Qt::DisplayRole) {
		const BuffListModelData &d = listData[index.row()];
		return d.group->levels[d.level]->getName(dataLanguage);
	} else {
		return QVariant();
	}
}

void BuffListModel::clear() {
	if (listData.count() > 0) {
		beginRemoveRows(QModelIndex(), 0, listData.count() - 1);
		listData.clear();
		listDataMap.clear();
		endRemoveRows();
	}
}

void BuffListModel::addBuff(const BuffGroup *group, int level) {
	if (level < 0) removeBuff(group, -level);
	if (level >= group->levels.count()) level = group->levels.count() - 1;
	if (level <= 0) return;

	QMap<const BuffGroup *, int>::iterator it = listDataMap.find(group);
	if (it == listDataMap.end()) {
		int new_row_idx = listData.count();
		beginInsertRows(QModelIndex(), new_row_idx, new_row_idx);
		listData.append(BuffListModelData(group, level));
		listDataMap[group] = new_row_idx;
		endInsertRows();
	} else {
		int new_level = listData[*it].level + level;
		if (new_level >= group->levels.count()) {
			new_level = group->levels.count() - 1;
		}
		if (new_level > listData[*it].level) {
			listData[*it].level = new_level;
			emit dataChanged(index(*it), index(*it));
		}
	}
}

void BuffListModel::removeBuff(const BuffGroup *group, int level) {
	if (level < 0) addBuff(group, -level);
	if (level <= 0) return;

	QMap<const BuffGroup *, int>::iterator it = listDataMap.find(group);
	if (it != listDataMap.end()) {
		int new_level = listData[*it].level - level;
		if (new_level <= 0) {
			beginRemoveRows(QModelIndex(), *it, *it);
			listDataMap.clear();
			listData.removeAt(*it);
			for (int i = 0; i < listData.count(); ++i) {
				listDataMap[listData[i].group] = i;
			}
			endRemoveRows();
		} else {
			listData[*it].level = new_level;
			emit dataChanged(index(*it), index(*it));
		}
	}
}

void BuffListModel::setDataLanguage(NamedObject::Language lang) {
	dataLanguage = lang;
	emit dataChanged(index(0), index(rowCount() - 1));
}

QMap<const BuffGroup *, int> BuffListModel::getBuffLevels() const {
	QMap<const BuffGroup *, int> ret;
	foreach(const BuffListModelData &d, listData) {
		ret[d.group] = d.level;
	};
	return ret;
}

const BuffGroup *BuffListModel::buffGroup(const QModelIndex &index) const {
	if (!index.isValid()) return NULL;
	if (index.row() < 0 && index.row() >= listData.count()) return NULL;
	return listData[index.row()].group;
}

QModelIndex BuffListModel::buffGroupIndex(const BuffGroup *group) const {
	QMap<const BuffGroup *, int>::const_iterator it =
		listDataMap.find(group);
	if (it == listDataMap.end()) {
		return QModelIndex();
	} else {
		return index(*it);
	}
}
