#ifndef NamedObject_h_
#define NamedObject_h_

#include <QString>
#include "enums.h"
#include "XmlObject.h"

class QTextStream;
class QXmlStreamReader;

struct NamedObject : public XmlObject {
	QString id;
	QString names[LANG_COUNT];

	virtual ~NamedObject() {}
	QString getName(Language lang) const;
	QString getAllNames() const;
	virtual void print(QTextStream &stream, QString indent = QString()) const;
	virtual bool readXmlElement(QXmlStreamReader *xml);
	virtual void readXml(QXmlStreamReader *xml);
};

#endif
