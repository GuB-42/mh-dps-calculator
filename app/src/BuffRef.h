#ifndef BuffRef_h_
#define BuffRef_h_

#include <QString>
#include "XmlObject.h"

class QTextStream;
class QXmlStreamReader;
struct BuffGroup;
struct BuffSetBonus;

struct BuffRef : XmlObject {
	BuffRef() : buffGroup(NULL), level(1) {};
	QString id;
	BuffGroup *buffGroup;
	int level;

	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml);
};

struct BuffSetBonusRef : XmlObject {
	BuffSetBonusRef() : buffSetBonus(NULL), level(1) {};
	QString id;
	BuffSetBonus *buffSetBonus;
	int level;

	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml);
};

#endif
