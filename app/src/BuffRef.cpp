#include "BuffRef.h"

#include <QTextStream>
#include <QXmlStreamReader>

void BuffRef::print(QTextStream &stream, QString indent) const {
	XmlObject::print(stream, indent);
	stream << indent << "- " << id << "[" << level << "]";
	if (buffGroup) stream << " *";
	stream << endl;
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
	stream << endl;
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
