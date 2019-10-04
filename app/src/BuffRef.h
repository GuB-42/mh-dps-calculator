#ifndef BuffRef_h_
#define BuffRef_h_

#include <QString>

class QTextStream;
class QXmlStreamReader;
struct BuffGroup;
struct BuffSetBonus;

struct BuffRef {
	BuffRef() : buffGroup(NULL), level(1), levelCapPlus(0) {};
	QString id;
	BuffGroup *buffGroup;
	int level;
	int levelCapPlus;

	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml);
};

struct BuffSetBonusRef {
	BuffSetBonusRef() : buffSetBonus(NULL), level(1) {};
	QString id;
	BuffSetBonus *buffSetBonus;
	int level;

	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml);
};

#endif
