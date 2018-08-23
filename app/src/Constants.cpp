#include "Constants.h"
#include <math.h>
#include "enums.h"
#include "ConditionRatios.h"

Constants::Constants() {
	ConditionRatios *cond = new ConditionRatios();

	rawCriticalHitMultiplier = 1.25;
	elementCriticalHitMultiplier = 1.0;
	statusCriticalHitMultiplier = 1.0;
	feebleHitMultiplier = 0.75;
	phialPowerBoost = 1.2;
	phialElementBoost = 1.25;
	statusAttackRate = (1.0 / 3.0);
	rawWeakSpotThreshold = 45.0;
	elementWeakSpotThreshold = 20.0;
	bounceThreshold = 25.0;
	piercingFactor = 0.72;
	impactPhialRawBonusMultiplier = 0.5;

	rawSharpnessMultipliers[SHARPNESS_RED] = 0.5;
	rawSharpnessMultipliers[SHARPNESS_ORANGE] = 0.75;
	rawSharpnessMultipliers[SHARPNESS_YELLOW] = 1.0;
	rawSharpnessMultipliers[SHARPNESS_GREEN] = 1.05;
	rawSharpnessMultipliers[SHARPNESS_BLUE] = 1.2;
	rawSharpnessMultipliers[SHARPNESS_WHITE] = 1.32;
	rawSharpnessMultipliers[SHARPNESS_PURPLE] = 1.44;

	elementSharpnessMultipliers[SHARPNESS_RED] = 0.25;
	elementSharpnessMultipliers[SHARPNESS_ORANGE] = 0.5;
	elementSharpnessMultipliers[SHARPNESS_YELLOW] = 0.75;
	elementSharpnessMultipliers[SHARPNESS_GREEN] = 1.0;
	elementSharpnessMultipliers[SHARPNESS_BLUE] = 1.0625;
	elementSharpnessMultipliers[SHARPNESS_WHITE] = 1.125;
	elementSharpnessMultipliers[SHARPNESS_PURPLE] = 1.2;

	QVector<BuffCap> bcaps;
	bcaps <<
		BuffCap( 1.0,  4.0) <<
		BuffCap(12.0,  4.0) <<
		BuffCap(15.0,  5.0) <<
		BuffCap(21.0,  6.0) <<
		BuffCap(24.0,  7.0) <<
		BuffCap(27.0,  8.0) <<
		BuffCap(30.0,  9.0) <<
		BuffCap(33.0, 10.0) <<
		BuffCap(36.0, 11.0) <<
		BuffCap(39.0, 12.0) <<
		BuffCap(42.0, 13.0) <<
		BuffCap(48.0, 14.0) <<
		BuffCap(54.0, 15.0);
	elementBuffCaps = bcaps;
	statusBuffCaps = bcaps;

	enragedRatio = 0.4;
	monsterDefenseMultiplier = 0.5;
	sharpenPeriod = 150.0;

	(*cond)[CONDITION_ALWAYS] = 1.0;
	(*cond)[CONDITION_ENRAGED] = 0.0;
	(*cond)[CONDITION_WEAK_SPOT] = 0.0;
	(*cond)[CONDITION_RAW_WEAPON] = 0.0;
	(*cond)[CONDITION_DRAW_ATTACK] = 0.1;
	(*cond)[CONDITION_AIRBORNE] = 0.1;
	(*cond)[CONDITION_RED_LIFE] = 0.2;
	(*cond)[CONDITION_FULL_LIFE] = 0.7;
	(*cond)[CONDITION_LOW_LIFE] = 0.05;
	(*cond)[CONDITION_DEATH_1] = 0.0;
	(*cond)[CONDITION_DEATH_2] = 0.0;
	(*cond)[CONDITION_FULL_STAMINA] = 0.6;
	(*cond)[CONDITION_SLIDING] = 0.1;
	(*cond)[CONDITION_RECEIVED_DAMAGE] = 0.5;
	(*cond)[CONDITION_DEMON_POWDER_USE] = 0.5;
	(*cond)[CONDITION_MIGHT_SEED_USE] = 0.5;
	(*cond)[CONDITION_MIGHT_PILL_USE] = 0.1;
	(*cond)[CONDITION_SONG_BASE] = 0.25;
	(*cond)[CONDITION_SONG_ENCORE] = 0.5;
	conditionRatios = cond;
}

inline static double pow_A(double x) {
	return x * x * x;
}

double Constants::statusDamageCurve(double x) {
	double ox = x - 0.5;
	double ofpx = ox - floor(ox);
	double a = pow_A(ofpx);
	double b = pow_A(1.0 - ofpx);
	double satxp = pow_A(x > 1.0 ? 1.0 : x);
	return (a / (a + b) + floor(ox)) * satxp + pow_A(x * 0.5) * (1 - satxp);
}

const Constants *Constants::instancePtr = NULL;
