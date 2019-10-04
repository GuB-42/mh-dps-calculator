#ifndef MainData_h_
#define MainData_h_

#include <QVector>
#include <QHash>
#include <QString>

class QTextStream;
class QXmlStreamReader;
struct WeaponType;
struct Ammo;
struct Monster;
struct Weapon;
struct BuffGroup;
struct Profile;
struct Item;
struct Target;
struct MotionValue;
struct BuffSetBonus;
struct Song;
struct Category;

struct MainData {
	QVector<WeaponType *> weaponTypes;
	QHash<QString, WeaponType *> weaponTypeHash;
	QVector<Ammo *> ammos;
	QHash<QString, Ammo *> ammoHash;
	QVector<Monster *> monsters;
	QVector<Weapon *> weapons;
	QVector<BuffGroup *> buffGroups;
	QHash<QString, BuffGroup *> buffGroupHash;
	QVector<BuffSetBonus *> buffSetBonuses;
	QHash<QString, BuffSetBonus *> buffSetBonusHash;
	QVector<Profile *> profiles;
	QVector<Item *> items;
	QHash<QString, Item *> itemHash;
	QVector<Target *> targets;
	QVector<MotionValue *> motionValues;
	QHash<QString, MotionValue *> motionValueHash;
	QVector<Song *> songs;
	QVector<Category *> categories;
	QHash<QString, Category *> categoryHash;

	~MainData();
	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml);
	void matchData();
};

#endif
