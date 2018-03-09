#ifndef Constants_h_
#define Constants_h_

class ConditionRatios;

struct Constants {
	double rawCriticalHitMultiplier;
	double elementCriticalHitMultiplier;
	double statusCriticalHitMultiplier;
	double feebleHitMultiplier;

	const ConditionRatios *conditionRatios;
	double enragedRatio;
	double monsterDefenseMultiplier;
	double sharpenPeriod;
	double sharpnessUse;

	Constants();
	static const Constants *instance();
private:
	static const Constants *instancePtr;
};

#endif
