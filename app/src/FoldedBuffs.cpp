#include "FoldedBuffs.h"

#include "BuffWithCondition.h"
#include "ConditionRatios.h"
#include "Constants.h"

void FoldedBuffsData::applyBuff(const BuffWithCondition *buff_cond,
                                const ConditionRatios &ratios,
                                bool enraged, bool weak_spot,
                                bool raw_weapon, double base_affinity) {
	double ratio = 0.0;
	switch (buff_cond->condition) {
	case CONDITION_ALWAYS:
		ratio = 1.0;
		break;
	case CONDITION_ENRAGED:
		ratio = enraged ? 1.0 : 0.0;
		break;
	case CONDITION_WEAK_SPOT:
		ratio = weak_spot ? 1.0 : 0.0;
		break;
	case CONDITION_RAW_WEAPON:
		ratio = raw_weapon ? 1.0 : 0.0;
		break;
	default:
		ratio = ratios[buff_cond->condition];
		break;
	}

	if (ratio == 0.0) return;

	double *ov = NULL;
	enum { OP_NONE, OP_AFFINITY, OP_PLUS, OP_MULTIPLY, OP_MAX } op = OP_NONE;

	switch (buff_cond->buffClass) {
	case BuffWithCondition::BUFF_CLASS_NONE:
		break;
	case BuffWithCondition::BUFF_CLASS_NORMAL:
		ov = &normalBuffs[buff_cond->normal.buff];
		switch (buff_cond->normal.buff) {
		case BUFF_ATTACK_PLUS: op = OP_PLUS; break;
		case BUFF_ATTACK_MULTIPLIER: op = OP_MULTIPLY; break;
		case BUFF_AFFINITY_PLUS: op = OP_AFFINITY; break;
		case BUFF_ALL_ELEMENTS_PLUS: op = OP_PLUS; break;
		case BUFF_ALL_ELEMENTS_MULTIPLIER: op = OP_MULTIPLY; break;
		case BUFF_ALL_STATUSES_PLUS: op = OP_PLUS; break;
		case BUFF_ALL_STATUSES_MULTIPLIER: op = OP_MULTIPLY; break;
		case BUFF_AWAKENING: op = OP_PLUS; break;
		case BUFF_SHARPNESS_PLUS: op = OP_PLUS; break;
		case BUFF_MAX_SHARPNESS_TIME: op = OP_PLUS; break;
		case BUFF_SHARPNESS_USE_MULTIPLIER: op = OP_MULTIPLY; break;
		case BUFF_SHARPNESS_USE_CRITICAL_MULTIPLIER: op = OP_MULTIPLY; break;
		case BUFF_DRAW_ATTACK_STUN: op = OP_PLUS; break;
		case BUFF_DRAW_ATTACK_EXHAUST: op = OP_PLUS; break;
		case BUFF_STUN_MULTIPLIER: op = OP_MULTIPLY; break;
		case BUFF_EXHAUST_MULTIPLIER: op = OP_MULTIPLY; break;
		case BUFF_ARTILLERY_MULTIPLIER: op = OP_MULTIPLY; break;
		case BUFF_RAW_CRITICAL_HIT_MULTIPLIER: op = OP_MAX; break;
		case BUFF_ELEMENT_CRITICAL_HIT_MULTIPLIER: op = OP_MAX; break;
		case BUFF_STATUS_CRITICAL_HIT_MULTIPLIER: op = OP_MAX; break;
		case BUFF_MINDS_EYE: op = OP_PLUS; break;
		case NORMAL_BUFF_COUNT: op = OP_NONE; break;
		}
		break;
	case BuffWithCondition::BUFF_CLASS_ELEMENT:
		ov = &elementBuffs[buff_cond->element.buff][buff_cond->element.type];
		switch (buff_cond->element.buff) {
		case BUFF_ELEMENT_PLUS: op = OP_PLUS; break;
		case BUFF_ELEMENT_MULTIPLIER: op = OP_MULTIPLY; break;
		case ELEMENT_BUFF_COUNT: op = OP_NONE; break;
		}
		break;
	case BuffWithCondition::BUFF_CLASS_STATUS:
		ov = &statusBuffs[buff_cond->status.buff][buff_cond->status.type];
		switch (buff_cond->status.buff) {
		case BUFF_STATUS_PLUS: op = OP_PLUS; break;
		case BUFF_STATUS_MULTIPLIER: op = OP_MULTIPLY; break;
		case ELEMENT_BUFF_COUNT: op = OP_NONE; break;
		}
		break;
	}
	if (ov) {
		double v = *ov;
		switch (op) {
		case OP_NONE:
			break;
		case OP_AFFINITY:
			v += buff_cond->value;
			if (v + base_affinity > 100.0) v = 100.0 - base_affinity;
			if (v + base_affinity < -100.0) v = -100.0 - base_affinity;
			break;
		case OP_PLUS:
			v += buff_cond->value;
			break;
		case OP_MULTIPLY:
			v *= buff_cond->value;
			break;
		case OP_MAX:
			v = v > buff_cond->value ? v : buff_cond->value;
			break;
		}
		*ov += (v - *ov) * ratio;
	}
}

FoldedBuffsData::FoldedBuffsData() {
	normalBuffs[BUFF_ATTACK_PLUS] = 0.0;
	normalBuffs[BUFF_ATTACK_MULTIPLIER] = 1.0;
	normalBuffs[BUFF_AFFINITY_PLUS] = 0.0;
	normalBuffs[BUFF_ALL_ELEMENTS_PLUS] = 0.0;
	normalBuffs[BUFF_ALL_ELEMENTS_MULTIPLIER] = 1.0;
	normalBuffs[BUFF_ALL_STATUSES_PLUS] = 0.0;
	normalBuffs[BUFF_ALL_STATUSES_MULTIPLIER] = 1.0;
	normalBuffs[BUFF_AWAKENING] = 0.0;
	normalBuffs[BUFF_SHARPNESS_PLUS] = 0.0;
	normalBuffs[BUFF_MAX_SHARPNESS_TIME] = 0.0;
	normalBuffs[BUFF_SHARPNESS_USE_MULTIPLIER] = 1.0;
	normalBuffs[BUFF_SHARPNESS_USE_CRITICAL_MULTIPLIER] = 1.0;
	normalBuffs[BUFF_DRAW_ATTACK_STUN] = 0.0;
	normalBuffs[BUFF_DRAW_ATTACK_EXHAUST] = 0.0;
	normalBuffs[BUFF_STUN_MULTIPLIER] = 1.0;
	normalBuffs[BUFF_EXHAUST_MULTIPLIER] = 1.0;
	normalBuffs[BUFF_ARTILLERY_MULTIPLIER] = 1.0;
	normalBuffs[BUFF_RAW_CRITICAL_HIT_MULTIPLIER] =
		Constants::instance()->rawCriticalHitMultiplier;
	normalBuffs[BUFF_ELEMENT_CRITICAL_HIT_MULTIPLIER] =
		Constants::instance()->elementCriticalHitMultiplier;
	normalBuffs[BUFF_STATUS_CRITICAL_HIT_MULTIPLIER] =
		Constants::instance()->statusCriticalHitMultiplier;
	normalBuffs[BUFF_MINDS_EYE] = 0.0;
	for (int i = 0; i < ELEMENT_COUNT; ++i) {
		elementBuffs[BUFF_ELEMENT_PLUS][i] = 0.0;
		elementBuffs[BUFF_ELEMENT_MULTIPLIER][i] = 1.0;
	}
	for (int i = 0; i < STATUS_COUNT; ++i) {
		statusBuffs[BUFF_STATUS_PLUS][i] = 0.0;
		statusBuffs[BUFF_STATUS_MULTIPLIER][i] = 1.0;
	}
}

template <int N>
struct FoldedBuffsWithData : public FoldedBuffs {
	FoldedBuffsData data[N];
};
static FoldedBuffs *alloc_data(bool has_rage, bool has_weak_spot) {
	if (has_rage) {
		if (has_weak_spot) {
			FoldedBuffsWithData<4> *tret = new FoldedBuffsWithData<4>();
			tret->notEnragedNormalSpot = &tret->data[0];
			tret->notEnragedWeakSpot = &tret->data[1];
			tret->enragedNormalSpot = &tret->data[2];
			tret->enragedWeakSpot = &tret->data[3];
			return tret;
		} else {
			FoldedBuffsWithData<2> *tret = new FoldedBuffsWithData<2>();
			tret->notEnragedNormalSpot = &tret->data[0];
			tret->notEnragedWeakSpot = &tret->data[0];
			tret->enragedNormalSpot = &tret->data[1];
			tret->enragedWeakSpot = &tret->data[1];
			return tret;
		}
	} else {
		if (has_weak_spot) {
			FoldedBuffsWithData<2> *tret = new FoldedBuffsWithData<2>();
			tret->notEnragedNormalSpot = &tret->data[0];
			tret->notEnragedWeakSpot = &tret->data[1];
			tret->enragedNormalSpot = &tret->data[0];
			tret->enragedWeakSpot = &tret->data[1];
			return tret;
		} else {
			FoldedBuffsWithData<1> *tret = new FoldedBuffsWithData<1>();
			tret->notEnragedNormalSpot = &tret->data[0];
			tret->notEnragedWeakSpot = &tret->data[0];
			tret->enragedNormalSpot = &tret->data[0];
			tret->enragedWeakSpot = &tret->data[0];
			return tret;
		}
	}
}

FoldedBuffs *computeFoldedBuffs(const QList<const BuffWithCondition *> &buff_conds,
                                const ConditionRatios &ratios,
                                bool raw_weapon, double base_affinity) {
	bool has_rage = false;
	bool has_weak_spot = false;
	foreach(const BuffWithCondition *bc, buff_conds) {
		if (bc->condition == CONDITION_ENRAGED) {
			has_rage = true;
		} else if (bc->condition == CONDITION_WEAK_SPOT) {
			has_weak_spot = true;
		}
	}
	FoldedBuffs *ret = alloc_data(has_rage, has_weak_spot);
	foreach(const BuffWithCondition *bc, buff_conds) {
		ret->notEnragedNormalSpot->applyBuff(bc, ratios, false, false,
		                                     raw_weapon, base_affinity);
		if (has_rage) {
			ret->enragedNormalSpot->applyBuff(bc, ratios, true, false,
			                                  raw_weapon, base_affinity);
		}
		if (has_weak_spot) {
			ret->notEnragedNormalSpot->applyBuff(bc, ratios, false, true,
			                                     raw_weapon, base_affinity);
			if (has_rage) {
				ret->enragedNormalSpot->applyBuff(bc, ratios, true, true,
				                                  raw_weapon, base_affinity);
			}
		}
	}
	return 0;
}
