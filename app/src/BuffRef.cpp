#include "BuffRef.h"

#include <QTextStream>
#include <QXmlStreamReader>
#include "QtCompatibility.h"

void BuffRef::print(QTextStream &stream, QString indent) const {
	XmlObject::print(stream, indent);
	stream << indent << "- " << id << "[" << level << "]";
	if (buffGroup) stream << " *";
	stream << ENDL;
}

void BuffRef::readXml(QXmlStreamReader *xml) {
	if (xml->attributes().hasAttribute("id")) {
		id = xml->attributes().value("id").toString();
	}
	if (xml->attributes().hasAttribute("level")) {
		level = xml->attributes().value("level").toString().toInt();
	}
	XmlObject::readXml(xml);
}

void BuffSetBonusRef::print(QTextStream &stream, QString indent) const {
	XmlObject::print(stream, indent);
	stream << indent << "- " << id << "[" << level << "]";
	if (buffSetBonus) stream << " *";
	stream << ENDL;
}

void BuffSetBonusRef::readXml(QXmlStreamReader *xml) {
	if (xml->attributes().hasAttribute("id")) {
		id = xml->attributes().value("id").toString();
	}
	if (xml->attributes().hasAttribute("level")) {
		level = xml->attributes().value("level").toString().toInt();
	}
	XmlObject::readXml(xml);
}
