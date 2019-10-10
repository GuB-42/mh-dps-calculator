#include "BuffRef.h"

#include <QTextStream>
#include <QXmlStreamReader>

void BuffRef::print(QTextStream &stream, QString indent) const {
	XmlObject::print(stream, indent);
	stream << indent << "- " << id << "[" << level << "]";
	if (levelCapPlus) stream << " (cap plus: " << levelCapPlus << ")";
	if (buffGroup) stream << " *";
	stream << endl;
}

bool BuffRef::readXmlElement(QXmlStreamReader *xml) {
	if (XmlObject::readXmlElement(xml)) return true;
	QStringRef tag_name = xml->name();
	if (tag_name == "level_cap_plus") {
		levelCapPlus = xml->readElementText().toInt();
	} else {
		return false;
	}
	return true;
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
