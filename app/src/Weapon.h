#ifndef Weapon_h_
#define Weapon_h_

#include <QVector>
#include "enums.h"
#include "NamedObject.h"

class QTextStream;
class QXmlStreamReader;

struct Weapon : public NamedObject {
	QString type;
	double attack;
	double affinity;
	bool awakened;
	double elements[ELEMENT_COUNT];
	double statuses[STATUS_COUNT];
	PhialType phial;
	double phialElements[ELEMENT_COUNT];
	double phialStatuses[STATUS_COUNT];
	double sharpness[SHARPNESS_COUNT];
	double sharpnessPlus;
	QVector<int> decorationSlots;
	bool final;
	int rare;
	double elementCritAdjustment;
	double statusCritAdjustment;
	int augmentations;

	Weapon();
	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml);
};

#endif

