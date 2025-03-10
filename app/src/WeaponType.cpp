#include "WeaponType.h"

#include <QTextStream>
#include <QXmlStreamReader>
#include "QtCompatibility.h"


WeaponType::WeaponType() :
	elementCritAdjustment(1.0), statusCritAdjustment(1.0)
{
}

void WeaponType::print(QTextStream &stream, QString indent) const {
	NamedObject::print(stream, indent);
	stream << indent << "- main_attack_type: " << mainAttackType << ENDL;
	stream << indent << "- element_crit_adjustment: " << elementCritAdjustment << ENDL;
	stream << indent << "- status_crit_adjustment: " << statusCritAdjustment << ENDL;
}

bool WeaponType::readXmlElement(QXmlStreamReader *xml) {
	if (NamedObject::readXmlElement(xml)) return true;
	QStringRef tag_name = xml->name();
	if (tag_name == "main_attack_type") {
		mainAttackType = xml->readElementText();
	} else if (tag_name == "element_crit_adjustment") {
		elementCritAdjustment = xml->readElementText().toDouble();
	} else if (tag_name == "status_crit_adjustment") {
		statusCritAdjustment = xml->readElementText().toDouble();
	} else {
		return false;
	}
	return true;
}
