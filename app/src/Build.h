#ifndef Build_h_
#define Build_h_

#include <QVector>
#include <QMap>
#include <QString>

class QTextStream;
struct BuffGroup;
struct Item;
struct BuffWithCondition;
struct Weapon;

struct Build {
	const Weapon *weapon;
	QVector<int> decorationSlots;
	QMap<const BuffGroup *, int> buffLevels;
	QVector<const Item *> usedItems;

	Build();
	void addItem(const Item *item, bool take_slot = false);
	void addWeapon(const Weapon *weapon);
	void getBuffWithConditions(QVector<const BuffWithCondition *> *pout) const;
	void fillSlots(QVector<Build *> *pout, const QVector<Item *> &items) const;
	QVector<Item *> listUsefulItems(const QVector<Item *> &items) const;
	void print(QTextStream &stream, QString indent = QString()) const;
};

#endif
