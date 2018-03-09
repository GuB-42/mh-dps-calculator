#ifndef FoldedBuffs_h_
#define FoldedBuffs_h_

#include <QList>
#include "enums.h"

class BuffWithCondition;
class ConditionRatios;

struct FoldedBuffsData {
	double normalBuffs[NORMAL_BUFF_COUNT];
	double elementBuffs[ELEMENT_BUFF_COUNT][ELEMENT_COUNT];
	double statusBuffs[STATUS_BUFF_COUNT][STATUS_COUNT];

	FoldedBuffsData();
	void applyBuff(const BuffWithCondition *buff_cond,
	               const ConditionRatios &ratios,
	               bool enraged, bool weak_spot,
	               bool raw_weapon, double base_affinity);
};

struct FoldedBuffs {
	FoldedBuffsData *enragedWeakSpot;
	FoldedBuffsData *notEnragedWeakSpot;
	FoldedBuffsData *enragedNormalSpot;
	FoldedBuffsData *notEnragedNormalSpot;
};

FoldedBuffs *computeFoldedBuffs(const QList<const BuffWithCondition *> &buff_conds,
                                const ConditionRatios &ratios,
                                bool raw_weapon, double base_affinity);

#endif
