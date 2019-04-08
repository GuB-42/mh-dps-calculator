#include "Ammo.h"

#include <QTextStream>
#include <QXmlStreamReader>
#include "QtCompatibility.h"

void Ammo::print(QTextStream &stream, QString indent) const {
	NamedObject::print(stream, indent);
}

void Ammo::readXml(QXmlStreamReader *xml) {
	if (xml->attributes().hasAttribute("id")) {
		id = xml->attributes().value("id").toString();
	}
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			QStringRef tag_name = xml->name();
			if (readXmlName(xml)) {
				; // name
			} else {
				XML_SKIP_CURRENT_ELEMENT(*xml);
			}
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
}
