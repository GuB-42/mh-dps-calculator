#ifndef BuffGroup_h_
#define BuffGroup_h_

#include <QVector>
#include "NamedObject.h"

class QTextStream;
class QXmlStreamReader;
class BuffWithCondition;

struct BuffGroupLevel : public NamedObject {
	QVector<BuffWithCondition *> buffs;

	~BuffGroupLevel();
	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml, int *plevel);
};

struct BuffGroup : public NamedObject {
	QVector<BuffGroupLevel *> levels;

	BuffGroup();
	~BuffGroup();
	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml);
private:
	QVector<BuffGroupLevel *> alloc_list;
};

#endif
