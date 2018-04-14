#ifndef Profile_h_
#define Profile_h_

#include <QVector>
#include <QSet>
#include <QString>
#include "NamedObject.h"
#include "enums.h"

class QTextStream;
class QXmlStreamReader;
struct ConditionRatios;

struct Pattern {
	double rate;
	double period;
	double cut;
	double impact;
	double piercing;
	double element;
	double status;
	double stun;
	double exhaust;
	double sharpnessMultiplier;
	double phialImpactAttack;
	double phialImpactStun;
	double phialImpactExhaust;
	double phialElementAttack;
	double phialRatio;
	double mindsEyeRatio;
	double sharpnessUse;
	const ConditionRatios *conditionRatios;

	Pattern();
	~Pattern();
	void print(QTextStream &stream, QString indent = QString()) const;
	void updateConditionRatio(Condition cond, double v);
	void readXml(QXmlStreamReader *xml, QSet<Condition> *po_cond);
private:
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
