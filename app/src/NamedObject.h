#ifndef NamedObject_h_
#define NamedObject_h_

#include <QString>

class QTextStream;
class QXmlStreamReader;

struct NamedObject {
	enum Language {
		LANG_EN, LANG_FR, LANG_JP, LANG_COUNT
	};

	QString id;
	QString names[LANG_COUNT];

	virtual ~NamedObject() {}
	QString getName(Language lang) const;
	QString getAllNames() const;
	virtual void print(QTextStream &stream, QString indent = QString()) const;
	bool readXmlName(QXmlStreamReader *xml);
};

const char *toString(NamedObject::Language t);

#endif
