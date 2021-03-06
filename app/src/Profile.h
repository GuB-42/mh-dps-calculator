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
struct WeaponType;
struct Ammo;

struct MotionValueRef {
	MotionValueRef() : multiplier(1.0), rawMultiplier(1.0) { }
	QString id;
	double multiplier;
	double rawMultiplier;
};

struct PatternAmmoRef {
	PatternAmmoRef() : ammo(NULL) {};
	QString id;
	Ammo *ammo;
};

struct Pattern : public XmlObject {
	double rate;
	double period;
	double usage;

	bool capacityUpFilter;
	bool capacityUpEnabled;
	QVector<PatternAmmoRef> ammoRefs;

	double cut;
	double impact;
	double piercing;
	double bullet;
	double shell;
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
	bool readXmlElement(QXmlStreamReader *xml);
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
	WeaponType *weaponType;
	QString weaponTypeRefId;
	QVector<Pattern *> patterns;
	double sharpenPeriod;
	double rate;

	Profile();
	~Profile();
	void print(QTextStream &stream, QString indent = QString()) const;
	bool readXmlElement(QXmlStreamReader *xml);
	void readXml(QXmlStreamReader *xml);
private:
	ConditionRatios *localRatios;
};

#endif
