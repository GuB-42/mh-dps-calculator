#ifndef Weapon_h_
#define Weapon_h_

#include <QVector>
#include <QSet>
#include "enums.h"
#include "Item.h"
#include "BuffRef.h"

class QTextStream;
class QXmlStreamReader;
struct WeaponType;
struct Ammo;
struct Song;
struct BuffGroup;

struct WeaponAmmoRef {
	WeaponAmmoRef() : ammo(NULL) {};
	QString id;
	Ammo *ammo;
};

struct Weapon : public Item {
	WeaponType *weaponType;
	QString weaponTypeRefId;
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
	int augmentations;
	QVector<Note> notes;
	QVector<WeaponAmmoRef> ammoRefs;
	QVector<Song *> songs;

	Weapon();
	void print(QTextStream &stream, QString indent = QString()) const;
	bool readXmlElement(QXmlStreamReader *xml);
	void readXml(QXmlStreamReader *xml);
};

#endif

