#include "Constants.h"
#include "enums.h"
#include "ConditionRatios.h"

Constants::Constants() {
	ConditionRatios *cond = new ConditionRatios();

	rawCriticalHitMultiplier = 1.25;
	elementCriticalHitMultiplier = 1.0;
	statusCriticalHitMultiplier = 1.0;
	feebleHitMultiplier = 0.75;

	enragedRatio = 0.4;
	monsterDefenseMultiplier = 0.5;
	sharpenPeriod = 300.0;
	sharpnessUse = 15.0;
	(*cond)[CONDITION_ALWAYS] = 1.0;
	(*cond)[CONDITION_ENRAGED] = 0.0;
	(*cond)[CONDITION_WEAK_SPOT] = 0.0;
	(*cond)[CONDITION_RAW_WEAPON] = 0.0;
	(*cond)[CONDITION_DRAW_ATTACK] = 0.1;
	(*cond)[CONDITION_AIRBORNE] = 0.1;
	(*cond)[CONDITION_RED_LIFE] = 0.2;
	(*cond)[CONDITION_FULL_LIFE] = 0.7;
	(*cond)[CONDITION_DEATH_1] = 0.0;
	(*cond)[CONDITION_DEATH_2] = 0.0;
	(*cond)[CONDITION_FULL_STAMINA] = 0.6;
	(*cond)[CONDITION_SLIDING] = 0.1;
	(*cond)[CONDITION_LOW_LIFE] = 0.05;
	(*cond)[CONDITION_RECEIVED_DAMAGE] = 0.5;
	(*cond)[CONDITION_MIGHT_SEED_USE] = 0.5;
	(*cond)[CONDITION_DEMON_POWDER_USE] = 0.5;
	(*cond)[CONDITION_MIGHT_PILL_USE] = 0.1;
	conditionRatios = cond;
}

const Constants *Constants::instance() {
	if (!instancePtr) instancePtr = new Constants();
	return instancePtr;
}

const Constants *Constants::instancePtr = NULL;
