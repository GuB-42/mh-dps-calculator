#ifndef XmlObject_h_
#define XmlObject_h_

#include <QString>
#include "enums.h"

class QTextStream;
class QXmlStreamReader;

struct XmlObject {
	virtual ~XmlObject() {}
	virtual void print(QTextStream & /* stream */,
	                   QString /* indent */ = QString()) const {}
	virtual bool readXmlElement(QXmlStreamReader * /* xml */) { return false; }
	virtual void readXml(QXmlStreamReader *xml);
};

#endif
