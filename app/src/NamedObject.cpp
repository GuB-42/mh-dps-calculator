#include "NamedObject.h"

#include <QTextStream>
#include <QXmlStreamReader>

const char *toString(NamedObject::Language t) {
	switch (t) {
	case NamedObject::LANG_EN: return "en";
	case NamedObject::LANG_FR: return "fr";
	case NamedObject::LANG_JP: return "jp";
	default: return "???";
	}
}

static QString tagName(NamedObject::Language t) {
	switch (t) {
	case NamedObject::LANG_EN: return "name_en";
	case NamedObject::LANG_FR: return "name_fr";
	case NamedObject::LANG_JP: return "name_jp";
	default: return QString();
	}
}

QString NamedObject::getName(Language lang) const {
	if (!names[lang].isNull()) return names[lang];
	for (int i = 0; i < LANG_COUNT; ++i) {
		if (!names[i].isNull()) return names[i];
	}
	return QString();
}

QString NamedObject::getAllNames() const {
	QString ret;
	for (int i = 0; i < LANG_COUNT; ++i) {
		if (!names[i].isNull()) {
			if (ret.isNull()) {
				ret = names[i];
			} else {
				ret += "/";
				ret += names[i];
			}
		}
	}
	return ret;
}

void NamedObject::print(QTextStream &stream, QString indent) const {
	for (int i = 0; i < LANG_COUNT; ++i) {
		if (!names[i].isNull()) {
			stream << indent << "- name (" <<
				toString((Language)i) << "): " << names[i] << endl;
		}
	}
}

bool NamedObject::readXmlName(QXmlStreamReader *xml) {
	if (xml->tokenType() == QXmlStreamReader::StartElement) {
		QStringRef tag_name = xml->name();
		if (tag_name == "name") {
			names[0] = xml->readElementText();
			return true;
		} else if (tag_name.startsWith("name_")) {
			for (int i = 0; i < LANG_COUNT; ++i) {
				if (tag_name == tagName((Language)i)) {
					names[i] = xml->readElementText();
					return true;
				}
			}
		}
	}
	return false;
}
