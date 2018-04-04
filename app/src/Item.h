#ifndef Item_h_
#define Item_h_

#include "NamedObject.h"
#include <QVector>

struct BuffGroup;

struct Item : public NamedObject {
	struct BuffRef {
		BuffRef() : buffGroup(NULL), level(1) {};
		QString id;
		BuffGroup *buffGroup;
		int level;
	};

	QString type;
	int decorationLevel;
	int weaponAugmentationLevel;
	int weaponSlotUpgrade;
	int rare;
	QVector<int> decorationSlots;
	QVector<BuffRef> buffRefs;

	Item();
	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml);
};

#endif
