#ifndef Target_h_
#define Target_h_

#include <QVector>
#include <QString>
#include "NamedObject.h"

class QTextStream;
class QXmlStreamReader;
struct MonsterHitData;
struct MonsterPart;
struct Monster;

struct TargetZone {
	double weight;
	double expectedEnragedRatio;
	double enragedRatio;
	const MonsterPart *part;
	const MonsterHitData *hitData;

	TargetZone();
	void print(QTextStream &stream, QString indent) const;
};

struct TargetMonster {
	double defenseMultiplier;
	double statusDefenseMultiplier;
	double statusHitMultiplier;
	const Monster *monster;
	QVector<TargetZone *> targetZones;

	TargetMonster();
	~TargetMonster();
	void print(QTextStream &stream, QString indent) const;
};

struct Target : public NamedObject {
	struct SubTarget {
		double weight;
		bool hasDefenseMultiplier;
		double defenseMultiplier;
		bool hasStatusDefenseMultiplier;
		double statusDefenseMultiplier;
		bool hasStatusHitMultiplier;
		double statusHitMultiplier;
		bool hasEnragedRatio;
		double enragedRatio;
		QString monsterId;
		QString partId;
		QString stateId;
		QVector<SubTarget *> subTargets;

		SubTarget();
		~SubTarget();
		void print(QTextStream &stream, QString indent) const;
		void readXml(QXmlStreamReader *xml);
	};

	QVector<TargetMonster *> targetMonsters;
	SubTarget rootSubTarget;

	~Target();
	void print(QTextStream &stream, QString indent) const;
	void readXml(QXmlStreamReader *xml);
	void matchMonsters(const QVector<Monster *> &monsters);
};

#endif
