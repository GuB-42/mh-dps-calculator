#ifndef BuffGroup_h_
#define BuffGroup_h_

#include <QVector>
#include "NamedObject.h"

class QTextStream;
class QXmlStreamReader;
struct BuffWithCondition;

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
	bool hasBuffs();
	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml);
private:
	QVector<BuffGroupLevel *> alloc_list;
};

struct BuffSetBonus : public NamedObject {
	struct Level {
		Level() : buffSetLevel(1), buffGroup(NULL), buffLevel(1) {}
		int buffSetLevel;
		QString buffId;
		BuffGroup *buffGroup;
		int buffLevel;
	};

	QVector<Level> levels;

	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml);
};

#endif
