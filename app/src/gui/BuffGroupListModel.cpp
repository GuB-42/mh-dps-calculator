#include "BuffGroupListModel.h"
#include "../BuffGroup.h"

BuffGroupListModel::BuffGroupListModel(QObject *parent) :
	QAbstractListModel(parent), dataLanguage(NamedObject::LANG_EN)
{
}

int BuffGroupListModel::rowCount(const QModelIndex &parent) const {
	if (parent.isValid()) return 0;
	return listData.count();
}

QVariant BuffGroupListModel::data(const QModelIndex &index, int role) const {
	if (!index.isValid()) return QVariant();
	if (index.row() < 0 && index.row() >= listData.count()) return QVariant();

	if (role == Qt::DisplayRole) {
		const BuffGroup *g = listData[index.row()];
		return g->getName(dataLanguage);
	} else {
		return QVariant();
	}
}

void BuffGroupListModel::clear() {
	if (listData.count() > 0) {
		beginRemoveRows(QModelIndex(), 0, listData.count() - 1);
		listData.clear();
		listDataMap.clear();
		endRemoveRows();
	}
}

void BuffGroupListModel::addBuffGroup(const BuffGroup *group) {
	QMap<const BuffGroup *, int>::iterator it = listDataMap.find(group);
	if (it == listDataMap.end()) {
		int new_row_idx = listData.count();
		beginInsertRows(QModelIndex(), new_row_idx, new_row_idx);
		listData.append(group);
		listDataMap[group] = new_row_idx;
		endInsertRows();
	}
}

void BuffGroupListModel::setDataLanguage(NamedObject::Language lang) {
	dataLanguage = lang;
	emit dataChanged(index(0), index(rowCount() - 1));
}

const BuffGroup *BuffGroupListModel::buffGroup(const QModelIndex &index) const {
	if (!index.isValid()) return NULL;
	if (index.row() < 0 && index.row() >= listData.count()) return NULL;
	return listData[index.row()];
}

QModelIndex BuffGroupListModel::buffGroupIndex(const BuffGroup *group) const {
	QMap<const BuffGroup *, int>::const_iterator it =
		listDataMap.find(group);
	if (it == listDataMap.end()) {
		return QModelIndex();
	} else {
		return index(*it);
	}
}
