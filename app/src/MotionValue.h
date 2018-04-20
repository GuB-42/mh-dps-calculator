#ifndef MotionValue_h_
#define MotionValue_h_

#include <QString>
#include "NamedObject.h"
#include "enums.h"

class QTextStream;
class QXmlStreamReader;

struct MotionValue : public NamedObject {
	QString type;
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
	double phialImpactAttack;
	double phialImpactStun;
	double phialImpactExhaust;
	double phialElementAttack;
	double sharpnessUse;
	double punishingDrawStun;
	double punishingDrawExhaust;

	double sharpnessMultiplier;
	double drawAttackRatio;
	double airborneRatio;
	double phialRatio;
	double mindsEyeRatio;

	bool definedSharpnessMultiplier;
	bool definedDrawAttackRatio;
	bool definedAirborneRatio;
	bool definedPhialRatio;
	bool definedMindsEyeRatio;

	MotionValue();
	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml);
};

#endif
