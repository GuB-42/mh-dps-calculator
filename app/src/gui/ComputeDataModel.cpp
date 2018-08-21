#include "ComputeDataModel.h"

#include "GuiElements.h"
#include "../Profile.h"
#include "../Target.h"
#include "../ConditionRatios.h"
#include "../Constants.h"
#include "../Monster.h"
#include "../WeaponType.h"

ComputeDataModel::ComputeDataModel(const Profile *p, const Target *t,
                                   QObject *parent) :
	QAbstractTableModel(parent),
	sharpenPeriod(0.0), defenseMultiplier(0.0),
	statusDefenseMultiplier(0.0), statusHitMultiplier(0.0)
{
	double total_weight = 0.0;
	foreach(const Pattern *pattern, p->patterns) {
		double weight = pattern->rate / pattern->period;
		total_weight += weight;
		for (int i = 0; i < CONDITION_COUNT; ++i) {
			ratios[(Condition)i] +=
				(*pattern->conditionRatios)[(Condition)i] * weight;
		}
	}
	if (total_weight != 0.0) {
		for (int i = 0; i < CONDITION_COUNT; ++i) {
			ratios[(Condition)i] /= total_weight;
		}
	}
	sharpenPeriod = p->sharpenPeriod;

	total_weight = 0.0;
	ratios[CONDITION_ENRAGED] = 0.0;
	ratios[CONDITION_WEAK_SPOT] = 0.0;
	foreach(const TargetMonster *target_monster, t->targetMonsters) {
		foreach(const TargetZone *zone, target_monster->targetZones) {
			total_weight += zone->weight;
			ratios[CONDITION_ENRAGED] += zone->weight *	zone->enragedRatio;
			if (p->weaponType && p->weaponType->mainAttackType == "impact") {
				if (zone->hitData->impact >
				    Constants::instance()->rawWeakSpotThreshold) {
					ratios[CONDITION_WEAK_SPOT] += zone->weight;
				}
			} else if (p->weaponType && p->weaponType->mainAttackType == "bullet") {
				if (zone->hitData->bullet >
				    Constants::instance()->rawWeakSpotThreshold) {
					ratios[CONDITION_WEAK_SPOT] += zone->weight;
				}
			} else {
				if (zone->hitData->cut >
				    Constants::instance()->rawWeakSpotThreshold) {
					ratios[CONDITION_WEAK_SPOT] += zone->weight;
				}
			}
			defenseMultiplier += zone->weight *
				target_monster->defenseMultiplier;
			statusDefenseMultiplier += zone->weight *
				target_monster->statusDefenseMultiplier;
			statusHitMultiplier += zone->weight *
				target_monster->statusHitMultiplier;
		}
	}
	if (total_weight != 0.0) {
		ratios[CONDITION_ENRAGED] /= total_weight;
		ratios[CONDITION_WEAK_SPOT] /= total_weight;
		defenseMultiplier /= total_weight;
		statusDefenseMultiplier /= total_weight;
		statusHitMultiplier /= total_weight;
	}
}

int ComputeDataModel::conditionRow(Condition c) {
	if (c <= CONDITION_ALWAYS) return -1;
	if (c == CONDITION_RAW_WEAPON) return -1;
	if (c > CONDITION_COUNT) return -1;
	if (c < CONDITION_RAW_WEAPON) return c - 1;
	return c - 2;
}

Condition ComputeDataModel::rowCondition(int row) {
	if (row < 0) return CONDITION_ALWAYS;
	if (row < CONDITION_RAW_WEAPON - 1) return (Condition)(row + 1);
	if (row < CONDITION_COUNT - 2) return (Condition)(row + 2);
	return CONDITION_COUNT;
}

int ComputeDataModel::rowCount(const QModelIndex &parent) const {
	if (parent.isValid()) return 0;
	return conditionRow(CONDITION_COUNT) + 4;
}

int ComputeDataModel::columnCount(const QModelIndex &parent) const {
	if (parent.isValid()) return 0;
	return 2;
}

QVariant ComputeDataModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Horizontal) {
		if (role == Qt::DisplayRole) {
			switch (section) {
			case 0: return tr("Parameter");
			case 1: return tr("Value");
			default: return QVariant();
			}
		} else {
			return QVariant();
		}
	} else {
		return QVariant();
	}
}

QVariant ComputeDataModel::data(const QModelIndex &index, int role) const {
	if (!index.isValid()) return QVariant();
	if (index.row() < 0 || index.row() >= rowCount()) return QVariant();

	if (role == Qt::DisplayRole) {
		Condition c = rowCondition(index.row());
		int xdata = -1;
		if (c >= CONDITION_COUNT) {
			xdata = index.row() - conditionRow(CONDITION_COUNT);
		}
		switch (index.column()) {
		case 0:
			switch (xdata) {
			case 0:
				return tr("Sharpen period");
			case 1:
				return tr("Monster defense");
			case 2:
				return tr("Monster status defense");
			case 3:
				return tr("Status buildup multiplier");
			default:
				return tr("Ratio: %1").arg(GuiElements::conditionName(c));
			}
		case 1:
			switch (xdata) {
			case 0:
				return tr("%1 s").arg(sharpenPeriod);
			case 1:
				return tr("%1x").arg(defenseMultiplier);
			case 2:
				return tr("%1x").arg(statusDefenseMultiplier);
			case 3:
				return tr("%1x").arg(statusHitMultiplier);
			default:
				return tr("%1%").arg(ratios[c] * 100.0, 0, 'f', 1);
			}
		default: return QVariant();
		}
	} else if (role == Qt::TextAlignmentRole) {
		switch (index.column()) {
		case 0: return QVariant();
		case 1: return QVariant(Qt::AlignRight | Qt::AlignVCenter);
		default: return QVariant();
		}
	} else {
		return QVariant();
	}
}
