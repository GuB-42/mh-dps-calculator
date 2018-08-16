#include "BuffWithCondition.h"

#include <QTextStream>
#include "Weapon.h"
#include "Profile.h"

BuffWithCondition::BuffWithCondition() :
	buffClass(BUFF_CLASS_NONE), condition(CONDITION_ALWAYS), value(0.0)
{
}

bool BuffWithCondition::isUseful(const Weapon &weapon, const Profile &profile) const {
	if (condition == CONDITION_RAW_WEAPON) {
		bool raw_weapon = true;
		for (int i = 0; i < ELEMENT_COUNT; ++i) {
			if (weapon.elements[i] > 0.0) raw_weapon = false;
		}
		for (int i = 0; i < STATUS_COUNT; ++i) {
			if (weapon.statuses[i] > 0.0) raw_weapon = false;
		}
		if (!raw_weapon && !weapon.awakened) return false;
	}

	switch (buffClass) {
	case BUFF_CLASS_NONE:
		return false;
	case BUFF_CLASS_NORMAL:
		switch (normal.buff) {
		case BUFF_AWAKENING:
			return weapon.awakened;
		case BUFF_SHARPNESS_PLUS:
			return weapon.sharpnessPlus > 0.0;
		case BUFF_ARTILLERY_MULTIPLIER:
			if (weapon.phial == PHIAL_IMPACT) return true;
			foreach(Pattern *pattern, profile.patterns) {
				if (pattern->shell != 0.0) return true;
			}
			return false;
		case BUFF_ALL_ELEMENTS_PLUS:
		case BUFF_ALL_ELEMENTS_MULTIPLIER:
			for (int i = 0; i < ELEMENT_COUNT; ++i) {
				if (weapon.elements[i] > 0.0 ||
				    weapon.phialElements[i] > 0.0) return true;
			}
			return false;
		case BUFF_ALL_STATUSES_PLUS:
		case BUFF_ALL_STATUSES_MULTIPLIER:
			for (int i = 0; i < STATUS_COUNT; ++i) {
				if (weapon.statuses[i] > 0.0 ||
				    weapon.phialStatuses[i] > 0.0) return true;
			}
			return false;
		case BUFF_CAPACITY_UP:
			foreach(Pattern *pattern, profile.patterns) {
				if (pattern->capacityUpFilter) return true;
			}
			return false;
		default:
			return true;
		}
	case BUFF_CLASS_ELEMENT:
		switch (element.buff) {
		case BUFF_ELEMENT_PLUS:
		case BUFF_ELEMENT_MULTIPLIER:
			return weapon.elements[element.type] > 0.0 ||
				weapon.phialElements[element.type] > 0.0;
		default:
			return true;
		}
	case BUFF_CLASS_STATUS:
		switch (status.buff) {
		case BUFF_STATUS_PLUS:
		case BUFF_STATUS_MULTIPLIER:
			return weapon.statuses[status.type] > 0.0 ||
				weapon.phialStatuses[status.type] > 0.0;
		default:
			return true;
		}
	}
	return false;
}

bool BuffWithCondition::sameBuffAs(const BuffWithCondition &o) const {
	if (buffClass != o.buffClass) return false;
	switch (buffClass) {
	case BUFF_CLASS_NONE:
		return true;
	case BUFF_CLASS_NORMAL:
		return normal.buff == o.normal.buff;
	case BUFF_CLASS_ELEMENT:
		return element.buff == o.element.buff &&
			element.type == o.element.type;
	case BUFF_CLASS_STATUS:
		return status.buff == o.status.buff &&
			status.type == o.status.type;
	}
	return false;
}

bool BuffWithCondition::operator<(const BuffWithCondition &o) const {
	if (buffClass < o.buffClass) return true;
	if (o.buffClass < buffClass) return false;

	switch (buffClass) {
	case BuffWithCondition::BUFF_CLASS_NONE:
		break;
	case BuffWithCondition::BUFF_CLASS_NORMAL:
		if (normal.buff < o.normal.buff) return true;
		if (o.normal.buff < normal.buff) return false;
		break;
	case BuffWithCondition::BUFF_CLASS_ELEMENT:
		if (element.buff < o.element.buff) return true;
		if (o.element.buff < element.buff) return false;
		if (element.type < o.element.type) return true;
		if (o.element.type < element.type) return false;
		break;
	case BuffWithCondition::BUFF_CLASS_STATUS:
		if (status.buff < o.status.buff) return true;
		if (o.status.buff < status.buff) return false;
		if (status.type < o.status.type) return true;
		if (o.status.type < status.type) return false;
		break;
	};

	if (condition < o.condition) return true;
	if (o.condition < condition) return false;

	return value < o.value;
}

BuffWithCondition::BuffCombineOp BuffWithCondition::combineOp() const {
	switch (buffClass) {
	case BuffWithCondition::BUFF_CLASS_NONE:
		break;
	case BuffWithCondition::BUFF_CLASS_NORMAL:
		switch (normal.buff) {
		case BUFF_ATTACK_PLUS: return OP_PLUS;
		case BUFF_ATTACK_MULTIPLIER: return OP_MULTIPLY;
		case BUFF_AFFINITY_PLUS: return OP_AFFINITY;
		case BUFF_ALL_ELEMENTS_PLUS: return OP_PLUS;
		case BUFF_ALL_ELEMENTS_MULTIPLIER: return OP_MULTIPLY;
		case BUFF_ALL_STATUSES_PLUS: return OP_PLUS;
		case BUFF_ALL_STATUSES_MULTIPLIER: return OP_MULTIPLY;
		case BUFF_AWAKENING: return OP_PLUS;
		case BUFF_SHARPNESS_PLUS: return OP_PLUS;
		case BUFF_MAX_SHARPNESS_TIME: return OP_PLUS;
		case BUFF_SHARPNESS_USE_MULTIPLIER: return OP_MULTIPLY;
		case BUFF_SHARPNESS_USE_CRITICAL_MULTIPLIER: return OP_MULTIPLY;
		case BUFF_PUNISHING_DRAW: return OP_PLUS;
		case BUFF_STUN_MULTIPLIER: return OP_MULTIPLY;
		case BUFF_EXHAUST_MULTIPLIER: return OP_MULTIPLY;
		case BUFF_MOUNT_MULTIPLIER: return OP_MULTIPLY;
		case BUFF_ARTILLERY_MULTIPLIER: return OP_MULTIPLY;
		case BUFF_RAW_CRITICAL_HIT_MULTIPLIER: return OP_MAX;
		case BUFF_ELEMENT_CRITICAL_HIT_MULTIPLIER: return OP_MAX;
		case BUFF_STATUS_CRITICAL_HIT_MULTIPLIER: return OP_MAX;
		case BUFF_MINDS_EYE: return OP_PLUS;
		case BUFF_ATTACK_PLUS_BEFORE: return OP_PLUS;
		case BUFF_CAPACITY_UP: return OP_PLUS;
		case NORMAL_BUFF_COUNT: return OP_NONE;
		}
	case BuffWithCondition::BUFF_CLASS_ELEMENT:
		switch (element.buff) {
		case BUFF_ELEMENT_PLUS: return OP_PLUS;
		case BUFF_ELEMENT_MULTIPLIER: return OP_MULTIPLY;
		case ELEMENT_BUFF_COUNT: return OP_NONE;
		}
	case BuffWithCondition::BUFF_CLASS_STATUS:
		switch (status.buff) {
		case BUFF_STATUS_PLUS: return OP_PLUS;
		case BUFF_STATUS_MULTIPLIER: return OP_MULTIPLY;
		case STATUS_BUFF_COUNT: return OP_NONE;
		}
	}
	return OP_NONE;
}

void BuffWithCondition::print(QTextStream &stream, QString indent) const {
	stream << indent << toString(condition) << ": ";
	switch (buffClass) {
	case BUFF_CLASS_NONE:
		stream << "<none>" << endl;
		break;
	case BUFF_CLASS_NORMAL:
		stream << toString(normal.buff) << ": " << value << endl;
		break;
	case BUFF_CLASS_ELEMENT:
		stream << toString(element.buff) << " / " <<
			toString(element.type) << ": " << value << endl;
		break;
	case BUFF_CLASS_STATUS:
		stream << toString(status.buff) << " / " <<
			toString(status.type) << ": " << value << endl;
		break;
	}
}

