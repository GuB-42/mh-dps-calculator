#ifndef Build_h_
#define Build_h_

#include <QVector>
#include <QMap>
#include <QString>

class QTextStream;
class BuffGroup;
class Item;
class BuffWithCondition;
class Weapon;

struct Build {
	QVector<int> decorationSlots;
	QMap<const BuffGroup *, int> buffLevels;
	QVector<const Item *> usedItems;

	void addItem(Item *item, bool take_slot = false);
	void addWeapon(Weapon *weapon);
	void getBuffWithConditions(QVector<const BuffWithCondition *> *pout) const;
	void fillSlots(QVector<Build *> *pout, const QVector<Item *> &items) const;
	void print(QTextStream &stream, QString indent = QString()) const;
};

#endif
