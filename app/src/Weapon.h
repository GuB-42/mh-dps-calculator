#ifndef Weapon_h_
#define Weapon_h_

#include <QList>
#include "enums.h"
#include "NamedObject.h"

class QTextStream;
class QXmlStreamReader;

struct Weapon : public NamedObject {
	enum PhialType {
		PHIAL_NONE,
		PHIAL_ELEMENT,
		PHIAL_IMPACT,
		PHIAL_POWER,
		PHIAL_COUNT
	};

	enum SharpnessLevel {
		SHARPNESS_RED,
		SHARPNESS_ORANGE,
		SHARPNESS_YELLOW,
		SHARPNESS_GREEN,
		SHARPNESS_BLUE,
		SHARPNESS_WHITE,
		SHARPNESS_PURPLE,
		SHARPNESS_COUNT
	};

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
	QList<int> decorationSlots;
	int rare;

	Weapon();
	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml);
};

const char *toString(Weapon::PhialType t);
const char *toString(Weapon::SharpnessLevel s);

#endif

