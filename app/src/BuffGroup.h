#ifndef BuffGroup_h_
#define BuffGroup_h_

#include <QList>
#include "NamedObject.h"

class QTextStream;
class QXmlStreamReader;
class BuffWithCondition;

struct BuffGroupLevel : public NamedObject {
	QList<BuffWithCondition *> buffs;

	~BuffGroupLevel();
	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml, int *plevel);
};

struct BuffGroup : public NamedObject {
	QList<BuffGroupLevel *> levels;

	BuffGroup();
	~BuffGroup();
	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml);
private:
	QList<BuffGroupLevel *> alloc_list;
};

#endif
