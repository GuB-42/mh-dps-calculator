#ifndef MainData_h_
#define MainData_h_

#include <QVector>
#include <QHash>
#include <QString>

class QTextStream;
class QXmlStreamReader;
struct Monster;
struct Weapon;
struct BuffGroup;
struct Profile;
struct Item;
struct Target;

struct MainData {
	QVector<Monster *> monsters;
	QVector<Weapon *> weapons;
	QVector<BuffGroup *> buffGroups;
	QHash<QString, BuffGroup *> buffGroupHash;
	QVector<Profile *> profiles;
	QVector<Item *> items;
	QHash<QString, Item *> itemHash;
	QVector<Target *> targets;

	~MainData();
	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml);
	void matchData();
};

#endif
