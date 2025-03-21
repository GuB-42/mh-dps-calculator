#include "NamedObject.h"

#include <QTextStream>
#include <QXmlStreamReader>
#include "QtCompatibility.h"

static QString tagName(Language t) {
	return QString("name_") + toString(t);
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
	XmlObject::print(stream, indent);
	if (!id.isNull()) {
		stream << indent << "- id: " << id << ENDL;
	}
	for (int i = 0; i < LANG_COUNT; ++i) {
		if (!names[i].isNull()) {
			stream << indent << "- name (" <<
				toString((Language)i) << "): " << names[i] << ENDL;
		}
	}
}

bool NamedObject::readXmlElement(QXmlStreamReader *xml) {
	if (XmlObject::readXmlElement(xml)) return true;
	QStringRef tag_name = xml->name();
	if (tag_name == "name") {
		names[0] = xml->readElementText();
		return true;
	} else if (tag_name.toString().startsWith("name_")) {
		for (int i = 0; i < LANG_COUNT; ++i) {
			if (tag_name == tagName((Language)i)) {
				names[i] = xml->readElementText();
				return true;
			}
		}
	}
	return false;
}

void NamedObject::readXml(QXmlStreamReader *xml) {
	if (xml->attributes().hasAttribute("id")) {
		id = xml->attributes().value("id").toString();
	}
	XmlObject::readXml(xml);
}
