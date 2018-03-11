#ifndef MainData_h_
#define MainData_h_

#include <QList>
#include <QHash>
#include <QString>

class QTextStream;
class QXmlStreamReader;
class Monster;
class Weapon;
class BuffGroup;
class Profile;
class Item;

struct MainData {
	QList<Monster *> monsters;
	QList<Weapon *> weapons;
	QList<BuffGroup *> buffGroups;
	QHash<QString, BuffGroup *> buffGroupHash;
	QList<Profile *> profiles;
	QList<Item *> items;
	QHash<QString, Item *> itemHash;

	~MainData();
	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml);
	void matchData();
};

#endif
