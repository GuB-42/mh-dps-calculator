#ifndef BuffGroup_h_
#define BuffGroup_h_

#include <QVector>
#include "NamedObject.h"
#include "BuffRef.h"

class QTextStream;
class QXmlStreamReader;
struct BuffWithCondition;

struct BuffGroupLevel : public NamedObject {
	QVector<BuffWithCondition *> buffs;
	QVector<BuffRef> buffUncaps;

	~BuffGroupLevel();
	void print(QTextStream &stream, QString indent = QString()) const;
	bool readXmlElement(QXmlStreamReader *xml);
	void readXmlGetLevel(QXmlStreamReader *xml, int *plevel);
};

struct BuffGroup : public NamedObject {
	static const int LEVEL_UNCAPPED = 99999;
	QVector<BuffGroupLevel *> levels;
	int levelCap;

	BuffGroup();
	~BuffGroup();
	bool hasBuffs();
	void print(QTextStream &stream, QString indent = QString()) const;
	bool readXmlElement(QXmlStreamReader *xml);
	void readXml(QXmlStreamReader *xml);
private:
	QVector<BuffGroupLevel *> alloc_list;
};

struct BuffSetBonus : public NamedObject {
	struct Level {
		Level() : buffSetLevel(1) {}
		int buffSetLevel;
		BuffRef buffRef;
	};

	QVector<Level> levels;

	void print(QTextStream &stream, QString indent = QString()) const;
	bool readXmlElement(QXmlStreamReader *xml);
};

#endif
