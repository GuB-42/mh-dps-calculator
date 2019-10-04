#include "BuffRef.h"

#include <QTextStream>
#include <QXmlStreamReader>
#include "QtCompatibility.h"

void BuffRef::print(QTextStream &stream, QString indent) const {
	stream << indent << "- " << id << "[" << level << "]";
	if (levelCapPlus) stream << " (cap plus: " << levelCapPlus << ")";
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
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			QStringRef tag_name = xml->name();
			if (tag_name == "level_cap_plus") {
				levelCapPlus = xml->readElementText().toInt();
			} else {
				XML_SKIP_CURRENT_ELEMENT(*xml);
			}
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
}

void BuffSetBonusRef::print(QTextStream &stream, QString indent) const {
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
	XML_SKIP_CURRENT_ELEMENT(*xml);
}
