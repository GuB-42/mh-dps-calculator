#include "WeaponType.h"

#include <QTextStream>
#include <QXmlStreamReader>


WeaponType::WeaponType() :
	elementCritAdjustment(1.0), statusCritAdjustment(1.0)
{
}

void WeaponType::print(QTextStream &stream, QString indent) const {
	NamedObject::print(stream, indent);
	stream << indent << "- main_attack_type: " << mainAttackType << endl;
	stream << indent << "- element_crit_adjustment: " << elementCritAdjustment << endl;
	stream << indent << "- status_crit_adjustment: " << statusCritAdjustment << endl;
}

void WeaponType::readXml(QXmlStreamReader *xml) {
	if (xml->attributes().hasAttribute("id")) {
		id = xml->attributes().value("id").toString();
	}
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			QStringRef tag_name = xml->name();
			if (readXmlName(xml)) {
				; // name
			} else if (tag_name == "main_attack_type") {
				mainAttackType = xml->readElementText();
			} else if (tag_name == "element_crit_adjustment") {
				elementCritAdjustment = xml->readElementText().toDouble();
			} else if (tag_name == "status_crit_adjustment") {
				statusCritAdjustment = xml->readElementText().toDouble();
			} else {
				xml->skipCurrentElement();
			}
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
}
