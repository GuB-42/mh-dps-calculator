#ifndef WeaponType_h_
#define WeaponType_h_

#include "NamedObject.h"

class QTextStream;
class QXmlStreamReader;

struct WeaponType : public NamedObject {
	QString mainAttackType;
	double elementCritAdjustment;
	double statusCritAdjustment;

	WeaponType();
	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml);
};

#endif
