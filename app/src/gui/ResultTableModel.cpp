#include "ResultTableModel.h"
#include "../BuildWithDps.h"
#include "../Build.h"
#include "../Dps.h"
#include "../Weapon.h"
#include "../Item.h"

ResultTableModel::ResultTableModel(QObject *parent) :
	QAbstractTableModel(parent), itemColumns(0)
{
}

ResultTableModel::~ResultTableModel() {
	clearData();
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
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
		if (section == 0) {
			return tr("Total");
		} else if (section == 1) {
			return tr("Raw");
		} else if (section == 2) {
			return tr("Element");
		} else if (section == 3) {
			return tr("Status");
		} else if (section == 4) {
			return tr("Fixed");
		} else if (section == 5) {
			return tr("Bounce");
		} else if (section == 6) {
			return tr("Weapon");
		} else if (section >= 7 && section < 7 + itemColumns) {
			return tr("Item");
		} else {
			return QVariant();
		}
	} else {
		return QVariant();
	}
}

QVariant ResultTableModel::data(const QModelIndex &index, int role) const {
	if (!index.isValid()) return QVariant();
	if (index.row() < 0 && index.row() >= resultData.count()) return QVariant();

	BuildWithDps *bwd = resultData[index.row()];
	Dps *dps = bwd->dps;
	if (role == Qt::DisplayRole) {
		if (index.column() == 0) {
			return dps->raw + dps->total_elements +
				dps->total_statuses + dps->fixed;
		} else if (index.column() == 1) {
			return dps->raw;
		} else if (index.column() == 2) {
			return dps->total_elements;
		} else if (index.column() == 3) {
			return dps->total_statuses;
		} else if (index.column() == 4) {
			return dps->fixed;
		} else if (index.column() == 5) {
			return dps->bounceRate;
		} else if (index.column() == 6) {
			return bwd->build->weapon->getName(dataLanguage);
		} else if (index.column() >= 7 &&
		           index.column() < 7 + bwd->build->usedItems.count()) {
			return bwd->build->usedItems[index.column() - 7]->
				getName(dataLanguage);
		} else {
			return QVariant();
		}
	} else {
		return QVariant();
	}
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

void ResultTableModel::clearData() {
	foreach(BuildWithDps *bwd, resultData) delete bwd;
	resultData.clear();
	itemColumns = 0;
}
