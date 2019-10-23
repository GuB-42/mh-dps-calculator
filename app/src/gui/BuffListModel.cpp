#include "BuffListModel.h"
#include <algorithm>
#include <QBrush>
#include <QSet>
#include "../BuffGroup.h"

BuffListModel::BuffListModel(QObject *parent) :
	QAbstractListModel(parent), dataLanguage(LANG_EN)
{
}

int BuffListModel::rowCount(const QModelIndex &parent) const {
	if (parent.isValid()) return 0;
	return listData.count();
}

QVariant BuffListModel::data(const QModelIndex &index, int role) const {
	if (!index.isValid()) return QVariant();
	if (index.row() < 0 && index.row() >= listData.count()) return QVariant();

	if (role != Qt::DisplayRole && role != Qt::ForegroundRole) return QVariant();

	const BuffWithLevel &d = listData[index.row()];
	int level = d.level;
	if (d.group->levelCap != BuffGroup::LEVEL_UNCAPPED &&
	    level > d.group->levelCap && !d.levelUncapped) {
		level = d.group->levelCap;
	}
	while (level > 0 && d.group->levels[level] == d.group->levels[level - 1]) {
		--level;
	}

	if (role == Qt::DisplayRole) {
		const BuffGroupLevel *group_level = d.group->levels[level];
		if (group_level) {
			if (level < d.level) {
				return tr("%1 (+%2)").arg(group_level->getName(dataLanguage)).
					arg(d.level - level);
			} else {
				return group_level->getName(dataLanguage);
			}
		} else {
			return tr("%1 [%2]").arg(d.group->getName(dataLanguage)).arg(level);
		}
	} else {
		if (d.group->levelCap != BuffGroup::LEVEL_UNCAPPED &&
		    d.level > d.group->levelCap) {
			if (level > d.group->levelCap) {
				return QBrush(Qt::darkMagenta);
			} else {
				return QBrush(Qt::darkRed);
			}
		} else {
			return QVariant();
		}
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
		listData.append(BuffWithLevel(group, level));
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
	updateCaps();
}

void BuffListModel::removeBuff(const BuffGroup *group, int level) {
	if (level < 0) addBuff(group, -level);
	if (level <= 0) return;

	QMap<const BuffGroup *, int>::iterator it = listDataMap.find(group);
	if (it != listDataMap.end()) {
		int idx = *it;
		int new_level = listData[idx].level - level;
		if (new_level <= 0) {
			beginRemoveRows(QModelIndex(), idx, idx);
			listDataMap.clear();
			listData.removeAt(idx);
			for (int i = 0; i < listData.count(); ++i) {
				listDataMap[listData[i].group] = i;
			}
			endRemoveRows();
		} else {
			listData[idx].level = new_level;
			emit dataChanged(index(idx), index(idx));
		}
	}
	updateCaps();
}

void BuffListModel::setDataLanguage(Language lang) {
	dataLanguage = lang;
	emit dataChanged(index(0), index(rowCount() - 1));
}

QVector<BuffWithLevel> BuffListModel::getBuffLevels() const {
	QVector<BuffWithLevel> ret;
	ret = listData.toVector();
	std::sort(ret.begin(), ret.end());
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

void BuffListModel::updateCaps() {
	QSet<const BuffGroup *> uncapped;

	foreach(const BuffWithLevel &bwl, listData) {
		const BuffGroupLevel *group_level = bwl.group->levels[bwl.level];
		if (group_level) {
			foreach(const BuffRef &buff_ref, group_level->buffUncaps) {
				if (buff_ref.buffGroup) uncapped.insert(buff_ref.buffGroup);
			}
		}
	}
	for (int i = 0; i < listData.count(); ++i) {
		if (uncapped.contains(listData[i].group)) {
			if (!listData[i].levelUncapped) {
				listData[i].levelUncapped = true;
				emit dataChanged(index(i), index(i));
			}
		} else {
			if (listData[i].levelUncapped) {
				listData[i].levelUncapped = false;
				emit dataChanged(index(i), index(i));
			}
		}
	}
}
