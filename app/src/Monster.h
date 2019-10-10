#ifndef Monster_h_
#define Monster_h_

#include <QVector>
#include "enums.h"
#include "NamedObject.h"

class QTextStream;
class QXmlStreamReader;

struct MonsterHitData : public XmlObject {
	QString state;
	double enragedState;
	double cut;
	double impact;
	double bullet;
	double element[ELEMENT_COUNT];
	double stun;

	MonsterHitData();
	void print(QTextStream &stream, QString indent = QString()) const;
	bool readXmlElement(QXmlStreamReader *xml);
};

struct MonsterPart : public NamedObject {
	QVector<MonsterHitData *> hitData;

	~MonsterPart();
	void print(QTextStream &stream, QString indent = QString()) const;
	bool readXmlElement(QXmlStreamReader *xml);
};

struct MonsterTolerance : public XmlObject {
	double initial;
	double plus;
	double max;
	double regenValue;
	double regenTick;
	double duration;
	double damage;
	StatusType status_type;

	MonsterTolerance();
	void print(QTextStream &stream, QString indent = QString()) const;
	bool readXmlElement(QXmlStreamReader *xml);
};

struct Monster : public NamedObject {
	MonsterTolerance *tolerances[STATUS_COUNT];
	QVector<MonsterPart *> parts;
	double hitPoints;

	Monster();
	~Monster();
	void print(QTextStream &stream, QString indent = QString()) const;
	bool readXmlElement(QXmlStreamReader *xml);
};

#endif
