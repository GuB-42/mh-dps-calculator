#include "BuffWithCondition.h"

#include <QTextStream>
#include "Weapon.h"

BuffWithCondition::BuffWithCondition() :
	buffClass(BUFF_CLASS_NONE), condition(CONDITION_ALWAYS), value(0.0)
{
}

bool BuffWithCondition::isUseful(const Weapon &weapon) {
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
			// TODO: gunlance
			return weapon.phial == PHIAL_IMPACT;
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

