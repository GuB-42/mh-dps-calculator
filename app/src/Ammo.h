#ifndef Ammo_h_
#define Ammo_h_

#include "NamedObject.h"

class QTextStream;
class QXmlStreamReader;

struct Ammo : public NamedObject {
	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml);
};

#endif
