#ifndef Item_h_
#define Item_h_

#include "NamedObject.h"
#include <QList>

class BuffGroup;

struct Item : public NamedObject {
	struct BuffRef {
		BuffRef() : buffGroup(NULL), level(1) {};
		QString id;
		BuffGroup *buffGroup;
		int level;
	};

	QString type;
	int decorationLevel;
	int rare;
	QList<BuffRef> buffRefs;

	Item();
	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml);
};

#endif
