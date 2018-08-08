#include "FoldedBuffs.h"

#include <QTextStream>
#include "BuffWithCondition.h"
#include "ConditionRatios.h"
#include "Constants.h"

void FoldedBuffsData::applyBuff(const BuffWithCondition *buff_cond,
                                const ConditionRatios &ratios,
                                bool enraged, bool weak_spot,
                                double raw_weapon_ratio, double base_affinity) {
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
		ratio = raw_weapon_ratio;
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
		case BUFF_PUNISHING_DRAW: op = OP_PLUS; break;
		case BUFF_STUN_MULTIPLIER: op = OP_MULTIPLY; break;
		case BUFF_EXHAUST_MULTIPLIER: op = OP_MULTIPLY; break;
		case BUFF_MOUNT_MULTIPLIER: op = OP_MULTIPLY; break;
		case BUFF_ARTILLERY_MULTIPLIER: op = OP_MULTIPLY; break;
		case BUFF_RAW_CRITICAL_HIT_MULTIPLIER: op = OP_MAX; break;
		case BUFF_ELEMENT_CRITICAL_HIT_MULTIPLIER: op = OP_MAX; break;
		case BUFF_STATUS_CRITICAL_HIT_MULTIPLIER: op = OP_MAX; break;
		case BUFF_MINDS_EYE: op = OP_PLUS; break;
		case BUFF_ATTACK_PLUS_BEFORE: op = OP_PLUS; break;
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
	normalBuffs[BUFF_PUNISHING_DRAW] = 0.0;
	normalBuffs[BUFF_STUN_MULTIPLIER] = 1.0;
	normalBuffs[BUFF_EXHAUST_MULTIPLIER] = 1.0;
	normalBuffs[BUFF_MOUNT_MULTIPLIER] = 1.0;
	normalBuffs[BUFF_ARTILLERY_MULTIPLIER] = 1.0;
	normalBuffs[BUFF_RAW_CRITICAL_HIT_MULTIPLIER] =
		Constants::instance()->rawCriticalHitMultiplier;
	normalBuffs[BUFF_ELEMENT_CRITICAL_HIT_MULTIPLIER] =
		Constants::instance()->elementCriticalHitMultiplier;
	normalBuffs[BUFF_STATUS_CRITICAL_HIT_MULTIPLIER] =
		Constants::instance()->statusCriticalHitMultiplier;
	normalBuffs[BUFF_MINDS_EYE] = 0.0;
	normalBuffs[BUFF_ATTACK_PLUS_BEFORE] = 0.0;
	for (int i = 0; i < ELEMENT_COUNT; ++i) {
		elementBuffs[BUFF_ELEMENT_PLUS][i] = 0.0;
		elementBuffs[BUFF_ELEMENT_MULTIPLIER][i] = 1.0;
	}
	for (int i = 0; i < STATUS_COUNT; ++i) {
		statusBuffs[BUFF_STATUS_PLUS][i] = 0.0;
		statusBuffs[BUFF_STATUS_MULTIPLIER][i] = 1.0;
	}
}

void FoldedBuffsData::print(QTextStream &stream, QString indent) const {
	for (int i = 0; i < NORMAL_BUFF_COUNT; ++i) {
		stream << indent << "- " << toString((NormalBuff)i) << ": " <<
			normalBuffs[i] << endl;
	}
	for (int i = 0; i < ELEMENT_BUFF_COUNT; ++i) {
		stream << indent << "- " << toString((ElementBuff)i) << ": [";
		for (int j = 0; j < ELEMENT_COUNT; ++j) {
			if (j > 0) stream << ", ";
			stream << elementBuffs[i][j];
		}
		stream << "]" << endl;
	}
	for (int i = 0; i < STATUS_BUFF_COUNT; ++i) {
		stream << indent << "- " << toString((StatusBuff)i) << ": [";
		for (int j = 0; j < STATUS_COUNT; ++j) {
			if (j > 0) stream << ", ";
			stream << statusBuffs[i][j];
		}
		stream << "]" << endl;
	}
}

void FoldedBuffs::allocate_data(bool has_rage, bool has_weak_spot) {
	if (has_rage) {
		if (has_weak_spot) {
			alloc_data = new FoldedBuffsData[4];
			data[MODE_NORMAL_NORMAL] = &alloc_data[0];
			data[MODE_NORMAL_WEAK_SPOT] = &alloc_data[1];
			data[MODE_ENRAGED_NORMAL] = &alloc_data[2];
			data[MODE_ENRAGED_WEAK_SPOT] = &alloc_data[3];
		} else {
			alloc_data = new FoldedBuffsData[2];
			data[MODE_NORMAL_NORMAL] = &alloc_data[0];
			data[MODE_NORMAL_WEAK_SPOT] = &alloc_data[0];
			data[MODE_ENRAGED_NORMAL] = &alloc_data[1];
			data[MODE_ENRAGED_WEAK_SPOT] = &alloc_data[1];
		}
	} else {
		if (has_weak_spot) {
			alloc_data = new FoldedBuffsData[2];
			data[MODE_NORMAL_NORMAL] = &alloc_data[0];
			data[MODE_NORMAL_WEAK_SPOT] = &alloc_data[1];
			data[MODE_ENRAGED_NORMAL] = &alloc_data[0];
			data[MODE_ENRAGED_WEAK_SPOT] = &alloc_data[1];
		} else {
			alloc_data = new FoldedBuffsData[1];
			data[MODE_NORMAL_NORMAL] = &alloc_data[0];
			data[MODE_NORMAL_WEAK_SPOT] = &alloc_data[0];
			data[MODE_ENRAGED_NORMAL] = &alloc_data[0];
			data[MODE_ENRAGED_WEAK_SPOT] = &alloc_data[0];
		}
	}
}

FoldedBuffs::FoldedBuffs(const QVector<const BuffWithCondition *> &buff_conds,
                         const ConditionRatios &ratios,
                         double raw_weapon, bool awakened_weapon,
                         double base_affinity) {
	bool has_rage = false;
	bool has_weak_spot = false;
	foreach(const BuffWithCondition *bc, buff_conds) {
		if (bc->condition == CONDITION_ENRAGED) {
			has_rage = true;
		} else if (bc->condition == CONDITION_WEAK_SPOT) {
			has_weak_spot = true;
		}
	}
	allocate_data(has_rage, has_weak_spot);
	double raw_weapon_ratio = 0.0;
	if (raw_weapon) {
		raw_weapon_ratio = 1.0;
	} else if (awakened_weapon) {
		raw_weapon_ratio = 1.0;
		// FIXME: Conditional awakening isn't handled, it shouldn't matter
		foreach(const BuffWithCondition *bc, buff_conds) {
			if (bc->buffClass == BuffWithCondition::BUFF_CLASS_NORMAL &&
			    bc->normal.buff == BUFF_AWAKENING &&
			    bc->condition == CONDITION_ALWAYS) {
				raw_weapon_ratio -= bc->value;
			}
		}
		if (raw_weapon_ratio < 0.0) raw_weapon_ratio = 0.0;
	}
	foreach(const BuffWithCondition *bc, buff_conds) {
		data[MODE_NORMAL_NORMAL]->
			applyBuff(bc, ratios, false, false,
			          raw_weapon_ratio, base_affinity);
		if (has_rage) {
			data[MODE_ENRAGED_NORMAL]->
				applyBuff(bc, ratios, true, false,
				          raw_weapon_ratio, base_affinity);
		}
		if (has_weak_spot) {
			data[MODE_NORMAL_WEAK_SPOT]->
				applyBuff(bc, ratios, false, true,
				          raw_weapon_ratio, base_affinity);
			if (has_rage) {
				data[MODE_ENRAGED_WEAK_SPOT]->
					applyBuff(bc, ratios, true, true,
					          raw_weapon_ratio, base_affinity);
			}
		}
	}
}

FoldedBuffs::~FoldedBuffs() {
	delete[] alloc_data;
}

void FoldedBuffs::print(QTextStream &stream, QString indent) const {
	bool found[MODE_COUNT] = { false };
	for (int i = 0; i < MODE_COUNT; ++i) {
		if (found[i]) continue;
		stream << indent << "- " << toString((MonsterMode)i) << endl;
		for (int j = i + 1; j < MODE_COUNT; ++j) {
			if (data[i] == data[j]) {
				stream << indent << "- " << toString((MonsterMode)j) << endl;
			}
			found[j] = true;
		}
		data[i]->print(stream, indent + "\t");
	}
}
