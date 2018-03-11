#ifndef Constants_h_
#define Constants_h_

#include "enums.h"
#include <QVector>

class ConditionRatios;

struct BuffCap {
	BuffCap() : base(0.0), cap(0.0) {};
	BuffCap(double b, double c) : base(b), cap(c) {};
	double base;
	double cap;
};

struct Constants {
	double rawCriticalHitMultiplier;
	double elementCriticalHitMultiplier;
	double statusCriticalHitMultiplier;
	double feebleHitMultiplier;
	double phialPowerBoost;
	double phialElementBoost;
	double statusAttackRate;
	double rawWeakSpotThreshold;
	double elementWeakSpotThreshold;
	double bounceThreshold;
	double piercingFactor;
	double rawSharpnessMultipliers[SHARPNESS_COUNT];
	double elementSharpnessMultipliers[SHARPNESS_COUNT];
	const QVector<BuffCap> *elementBuffCaps;
	const QVector<BuffCap> *statusBuffCaps;

	const ConditionRatios *conditionRatios;
	double enragedRatio;
	double monsterDefenseMultiplier;
	double sharpenPeriod;
	double sharpnessUse;

	Constants();
	~Constants();
	inline static const Constants *instance() {
		if (!instancePtr) instancePtr = new Constants();
		return instancePtr;
	}
private:
	static const Constants *instancePtr;
};

#endif
