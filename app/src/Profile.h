#ifndef Profile_h_
#define Profile_h_

#include <QVector>
#include <QString>
#include <QHash>
#include <bitset>
#include "NamedObject.h"
#include "enums.h"

class QTextStream;
class QXmlStreamReader;
struct ConditionRatios;
struct MotionValue;

struct MotionValueRef {
	MotionValueRef() : multiplier(1.0), rawMultiplier(1.0) { }
	QString id;
	double multiplier;
	double rawMultiplier;
};

struct Pattern {
	double rate;
	double period;
	double cut;
	double impact;
	double piercing;
	double bullet;
	double fixed;
	double element;
	double status;
	double stun;
	double exhaust;
	double mount;
	double sharpnessMultiplier;
	double phialImpactAttack;
	double phialImpactStun;
	double phialImpactExhaust;
	double phialElementAttack;
	double phialRatio;
	double mindsEyeRatio;
	double sharpnessUse;
	double punishingDrawStun;
	double punishingDrawExhaust;

	QVector<MotionValueRef> motionValueRefs;
	bool definedPhialRatio;
	bool definedMindsEyeRatio;
	bool definedSharpnessMultiplier;
	std::bitset<CONDITION_COUNT> definedConditionRatios;
	const ConditionRatios *conditionRatios;

	Pattern();
	~Pattern();
	void print(QTextStream &stream, QString indent = QString()) const;
	void updateConditionRatio(Condition cond, double v);
	void applyMotionValues(const QHash<QString, MotionValue *> &mv_hash);
	void readXml(QXmlStreamReader *xml);
private:
	struct MultiplierTotals {
		MultiplierTotals() :
			sharpnessMultiplier(0.0),
			drawAttackRatio(0.0),
			airborneRatio(0.0),
			phialRatio(0.0),
			mindsEyeRatio(0.0) { }
		double sharpnessMultiplier;
		double drawAttackRatio;
		double airborneRatio;
		double phialRatio;
		double mindsEyeRatio;
	};
	void applyMotionValue(const MotionValue *mv,
	                      double multiplier, double raw_multiplier,
	                      const MultiplierTotals &totals);
	ConditionRatios *localRatios;
};

struct Profile : public NamedObject {
	QString type;
	QVector<Pattern *> patterns;
	double sharpenPeriod;

	Profile();
	~Profile();
	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml);
private:
	ConditionRatios *localRatios;
};

#endif
