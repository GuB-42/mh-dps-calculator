#ifndef BuffedWeapon_h_
#define BuffedWeapon_h_

#include "enums.h"
#include "Weapon.h"

class FoldedBuffsData;
class Weapon;

struct SharpnessMultiplierData {
	double rate;
	double multiplier;
};

struct BuffedWeapon {
	BuffedWeapon(const Weapon &weapon, const FoldedBuffsData &buffs,
	             double sharpen_period, double sharpness_use,
	             double element_status_crit_adjustment);
	double affinity;
	double attack;
	double elements[ELEMENT_COUNT];
	double statuses[STATUS_COUNT];
	double phialElements[ELEMENT_COUNT];
	double phialStatuses[STATUS_COUNT];
	Weapon::PhialType phial;
	double rawCriticalHitMultiplier;
	double elementCriticalHitMultiplier;
	double statusCriticalHitMultiplier;
	double drawAttackStun;
	double drawAttackExhaust;
	double stunMultiplier;
	double exhaustMultiplier;
	double artilleryMultiplier;
	double mindsEyeRatio;
	SharpnessMultiplierData bounceSharpness[Weapon::SHARPNESS_COUNT];
};

#endif
