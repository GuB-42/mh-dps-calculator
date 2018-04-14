#ifndef Constants_h_
#define Constants_h_

#include "enums.h"
#include <QVector>
#include <QHash>
#include <QString>

struct ConditionRatios;

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
	QVector<BuffCap> elementBuffCaps;
	QVector<BuffCap> statusBuffCaps;
	QHash<QString, double> elementCritAdjustment;
	QHash<QString, double> statusCritAdjustment;

	const ConditionRatios *conditionRatios;
	double enragedRatio;
	double monsterDefenseMultiplier;
	double sharpenPeriod;

	Constants();
	inline static const Constants *instance() {
		if (!instancePtr) instancePtr = new Constants();
		return instancePtr;
	}
private:
	static const Constants *instancePtr;
};

#endif
