#ifndef Item_h_
#define Item_h_

#include "NamedObject.h"
#include <QVector>
#include "BuffRef.h"

struct BuffGroup;
struct BuffSetBonus;

struct Item : public NamedObject {
	QString type;
	int decorationLevel;
	int weaponAugmentationLevel;
	int weaponSlotUpgrade;
	int rare;
	QVector<int> decorationSlots;
	QVector<BuffRef> buffRefs;
	QVector<BuffSetBonusRef> buffSetBonusRefs;

	Item();
	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml);
};

#endif
