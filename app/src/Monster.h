#ifndef Monster_h_
#define Monster_h_

#include <QList>
#include "enums.h"
#include "NamedObject.h"

class QTextStream;
class QXmlStreamReader;

struct MonsterHitData {
	QString state;
	double cut;
	double impact;
	double bullet;
	double element[ELEMENT_COUNT];
	double stun;

	MonsterHitData();
	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml);
};

struct MonsterPart : public NamedObject {
	QList<MonsterHitData *> hitData;

	~MonsterPart();
	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml);
};

struct MonsterTolerance {
	double initial;
	double plus;
	double max;
	double regen_value;
	double regen_tick;
	double duration;
	double damage;

	MonsterTolerance();
	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml, StatusType *pstatus_type);
};

struct Monster : public NamedObject {
	MonsterTolerance *tolerances[STATUS_COUNT];
	QList<MonsterPart *> parts;
	double hitPoints;

	Monster();
	~Monster();
	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml);
};

#endif
