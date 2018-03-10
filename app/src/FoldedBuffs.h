#ifndef FoldedBuffs_h_
#define FoldedBuffs_h_

#include <QVector>
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
	               double raw_weapon_ratio,
	               double base_affinity);
};

struct FoldedBuffs {
	FoldedBuffs(const QVector<const BuffWithCondition *> &buff_conds,
                const ConditionRatios &ratios,
                double raw_weapon, bool awakened_weapon,
                double base_affinity);
	~FoldedBuffs();
	FoldedBuffsData *data[MODE_COUNT];
private:
	void allocate_data(bool has_rage, bool has_weak_spot);
	FoldedBuffsData *alloc_data;
};

#endif
