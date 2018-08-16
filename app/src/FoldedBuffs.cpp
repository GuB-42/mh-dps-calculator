#include "FoldedBuffs.h"

#include <QTextStream>
#include "BuffWithCondition.h"
#include "ConditionRatios.h"
#include "Constants.h"

static bool exclusive_conditions(Condition c1, Condition c2) {
	switch (c1) {
	case CONDITION_RED_LIFE:
		return c2 == CONDITION_FULL_LIFE;
	case CONDITION_FULL_LIFE:
		return c2 == CONDITION_RED_LIFE || c2 == CONDITION_LOW_LIFE;
	case CONDITION_LOW_LIFE:
		return c2 == CONDITION_FULL_LIFE;
	case CONDITION_DEATH_1:
		return c2 == CONDITION_DEATH_2;
	case CONDITION_DEATH_2:
		return c2 == CONDITION_DEATH_1;
	case CONDITION_MIGHT_SEED_USE:
		return c2 == CONDITION_MIGHT_PILL_USE;
	case CONDITION_MIGHT_PILL_USE:
		return c2 == CONDITION_MIGHT_SEED_USE;
	case CONDITION_SONG_BASE:
		return c2 == CONDITION_SONG_ENCORE;
	case CONDITION_SONG_ENCORE:
		return c2 == CONDITION_SONG_BASE;
	default: return false;
	}
}

void FoldedBuffsData::applyBuff(const BuffWithCondition &buff_cond,
                                const ConditionRatios &ratios,
                                bool enraged, bool weak_spot,
                                double raw_weapon_ratio, double base_affinity,
                                double *last_value, bool use_last_value) {
	double ratio = 0.0;
	switch (buff_cond.condition) {
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
		ratio = ratios[buff_cond.condition];
		break;
	}

	if (ratio == 0.0) return;

	double *ov = NULL;
	switch (buff_cond.buffClass) {
	case BuffWithCondition::BUFF_CLASS_NONE:
		break;
	case BuffWithCondition::BUFF_CLASS_NORMAL:
		ov = &normalBuffs[buff_cond.normal.buff];
		break;
	case BuffWithCondition::BUFF_CLASS_ELEMENT:
		ov = &elementBuffs[buff_cond.element.buff][buff_cond.element.type];
		break;
	case BuffWithCondition::BUFF_CLASS_STATUS:
		ov = &statusBuffs[buff_cond.status.buff][buff_cond.status.type];
		break;
	}

	if (ov) {
		double last_v = last_value && use_last_value ? *last_value : *ov;
		double v = last_v;
		switch (buff_cond.combineOp()) {
		case BuffWithCondition::OP_NONE:
			break;
		case BuffWithCondition::OP_AFFINITY:
			v += buff_cond.value;
			if (v + base_affinity > 100.0) v = 100.0 - base_affinity;
			if (v + base_affinity < -100.0) v = -100.0 - base_affinity;
			break;
		case BuffWithCondition::OP_PLUS:
			v += buff_cond.value;
			break;
		case BuffWithCondition::OP_MULTIPLY:
			v *= buff_cond.value;
			break;
		case BuffWithCondition::OP_MAX:
			v = v > buff_cond.value ? v : buff_cond.value;
			break;
		}
		if (last_value) *last_value = last_v;
		*ov += (v - last_v) * ratio;
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
	normalBuffs[BUFF_CAPACITY_UP] = 0.0;
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
                         bool raw_weapon, bool awakened_weapon,
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

	double last_values[MODE_COUNT];
	for (int i = 0; i < buff_conds.count(); ) {
		bool use_last_value = i > 0 &&
			buff_conds[i]->sameBuffAs(*buff_conds[i - 1]) &&
			exclusive_conditions(buff_conds[i]->condition,
			                     buff_conds[i - 1]->condition);

		int next_i = i + 1;
		while (next_i < buff_conds.count() &&
		       buff_conds[i]->sameBuffAs(*buff_conds[next_i]) &&
		       buff_conds[i]->condition == buff_conds[next_i]->condition) {
			++next_i;
		}

		BuffWithCondition bc = *buff_conds[i];
		for (int j = i + 1; j < next_i; ++j) {
			switch (buff_conds[j]->combineOp()) {
			case BuffWithCondition::OP_NONE:
				break;
			case BuffWithCondition::OP_AFFINITY:
				bc.value += buff_conds[j]->value;
				break;
			case BuffWithCondition::OP_PLUS:
				bc.value += buff_conds[j]->value;
				break;
			case BuffWithCondition::OP_MULTIPLY:
				bc.value *= buff_conds[j]->value;
				break;
			case BuffWithCondition::OP_MAX:
				if (buff_conds[j]->value > bc.value) {
					bc.value = buff_conds[j]->value;
				}
				break;
			}
		}

		data[MODE_NORMAL_NORMAL]->
			applyBuff(bc, ratios, false, false,
			          raw_weapon_ratio, base_affinity,
			          &last_values[MODE_NORMAL_NORMAL],
			          use_last_value);
		if (has_rage) {
			data[MODE_ENRAGED_NORMAL]->
				applyBuff(bc, ratios, true, false,
				          raw_weapon_ratio, base_affinity,
				          &last_values[MODE_ENRAGED_NORMAL],
				          use_last_value);
		}
		if (has_weak_spot) {
			data[MODE_NORMAL_WEAK_SPOT]->
				applyBuff(bc, ratios, false, true,
				          raw_weapon_ratio, base_affinity,
				          &last_values[MODE_NORMAL_WEAK_SPOT],
				          use_last_value);
			if (has_rage) {
				data[MODE_ENRAGED_WEAK_SPOT]->
					applyBuff(bc, ratios, true, true,
					          raw_weapon_ratio, base_affinity,
					          &last_values[MODE_ENRAGED_WEAK_SPOT],
					          use_last_value);
			}
		}

		i = next_i;
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
