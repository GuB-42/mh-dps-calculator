#ifndef NamedObject_h_
#define NamedObject_h_

#include <QString>
#include "enums.h"

class QTextStream;
class QXmlStreamReader;

struct NamedObject {
	QString id;
	QString names[LANG_COUNT];

	virtual ~NamedObject() {}
	QString getName(Language lang) const;
	QString getAllNames() const;
	virtual void print(QTextStream &stream, QString indent = QString()) const;
	bool readXmlName(QXmlStreamReader *xml);
};

#endif
