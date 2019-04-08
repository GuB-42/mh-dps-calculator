#include "MotionValue.h"

#include <QTextStream>
#include <QXmlStreamReader>
#include "QtCompatibility.h"
#include "WeaponType.h"

MotionValue::MotionValue() :
	weaponType(NULL),
	cut(0.0),
	impact(0.0),
	piercing(0.0),
	bullet(0.0),
	shell(0.0),
	fixed(0.0),
	element(0.0),
	status(0.0),
	stun(0.0),
	exhaust(0.0),
	mount(0.0),
	phialImpactAttack(0.0),
	phialImpactStun(0.0),
	phialImpactExhaust(0.0),
	phialElementAttack(0.0),
	sharpnessUse(0.0),
	punishingDrawStun(0.0),
	punishingDrawExhaust(0.0),
	sharpnessMultiplier(1.0),
	drawAttackRatio(0.0),
	airborneRatio(0.0),
	phialRatio(0.0),
	mindsEyeRatio(0.0),
	definedSharpnessMultiplier(false),
	definedDrawAttackRatio(false),
	definedAirborneRatio(false),
	definedPhialRatio(false),
	definedMindsEyeRatio(false)
{
}

void MotionValue::print(QTextStream &stream, QString indent) const {
	NamedObject::print(stream, indent);
	stream << indent << "- weapon_type: ";
	if (weaponType) {
		stream << weaponType->id << endl;
	} else {
		stream << "<null, " << weaponTypeRefId << ">" << endl;
	}
	stream << indent << "- cut: " << cut << endl;
	stream << indent << "- impact: " << impact << endl;
	stream << indent << "- piercing: " << piercing << endl;
	stream << indent << "- bullet: " << bullet << endl;
	stream << indent << "- shell: " << shell << endl;
	stream << indent << "- fixed: " << fixed << endl;
	stream << indent << "- element: " << element << endl;
	stream << indent << "- status: " << status << endl;
	stream << indent << "- stun: " << stun << endl;
	stream << indent << "- exhaust: " << exhaust << endl;
	stream << indent << "- mount: " << mount << endl;
	stream << indent << "- phial_impact_attack: " << phialImpactAttack << endl;
	stream << indent << "- phial_impact_stun: " << phialImpactStun << endl;
	stream << indent << "- phial_impact_exhaust: " << phialImpactExhaust << endl;
	stream << indent << "- phial_element_attack: " << phialElementAttack << endl;
	stream << indent << "- sharpness_use: " << sharpnessUse << endl;
	stream << indent << "- punishing_draw_stun: " << punishingDrawStun << endl;
	stream << indent << "- punishing_draw_exhaust: " << punishingDrawExhaust << endl;
	if (definedSharpnessMultiplier) {
		stream << indent << "- sharpness_multiplier: " << sharpnessMultiplier << endl;
	}
	if (definedDrawAttackRatio) {
		stream << indent << "- draw_attack_ratio: " << drawAttackRatio << endl;
	}
	if (definedAirborneRatio) {
		stream << indent << "- airborne_ratio: " << airborneRatio << endl;
	}
	if (definedPhialRatio) {
		stream << indent << "- phial_ratio: " << phialRatio << endl;
	}
	if (definedMindsEyeRatio) {
		stream << indent << "- minds_eye_ratio: " << mindsEyeRatio << endl;
	}
}

void MotionValue::readXml(QXmlStreamReader *xml) {
	if (xml->attributes().hasAttribute("id")) {
		id = xml->attributes().value("id").toString();
	}
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			QStringRef tag_name = xml->name();
			if (readXmlName(xml)) {
				; // name
			} else if (tag_name == "weapon_type_ref") {
				weaponTypeRefId = xml->attributes().value("id").toString();
				XML_SKIP_CURRENT_ELEMENT(*xml);
			} else if (tag_name == "cut") {
				cut = xml->readElementText().toDouble();
			} else if (tag_name == "impact") {
				impact = xml->readElementText().toDouble();
			} else if (tag_name == "piercing") {
				piercing = xml->readElementText().toDouble();
			} else if (tag_name == "bullet") {
				bullet = xml->readElementText().toDouble();
			} else if (tag_name == "shell") {
				shell = xml->readElementText().toDouble();
			} else if (tag_name == "fixed") {
				fixed = xml->readElementText().toDouble();
			} else if (tag_name == "element") {
				status = element = xml->readElementText().toDouble();
//			} else if (tag_name == "status") {
//				status = xml->readElementText().toDouble();
			} else if (tag_name == "stun") {
				stun = xml->readElementText().toDouble();
			} else if (tag_name == "exhaust") {
				exhaust = xml->readElementText().toDouble();
			} else if (tag_name == "phial_impact_attack") {
				phialImpactAttack = xml->readElementText().toDouble();
			} else if (tag_name == "phial_impact_stun") {
				phialImpactStun = xml->readElementText().toDouble();
			} else if (tag_name == "phial_impact_exhaust") {
				phialImpactExhaust = xml->readElementText().toDouble();
			} else if (tag_name == "phial_element_attack") {
				phialElementAttack = xml->readElementText().toDouble();
			} else if (tag_name == "sharpness_use") {
				sharpnessUse = xml->readElementText().toDouble();
			} else if (tag_name == "punishing_draw_stun") {
				punishingDrawStun = xml->readElementText().toDouble();
			} else if (tag_name == "punishing_draw_exhaust") {
				punishingDrawExhaust = xml->readElementText().toDouble();
			} else if (tag_name == "sharpness_multiplier") {
				definedSharpnessMultiplier = true;
				sharpnessMultiplier = xml->readElementText().toDouble();
			} else if (tag_name == "draw_attack_ratio") {
				definedDrawAttackRatio = true;
				drawAttackRatio = xml->readElementText().toDouble();
			} else if (tag_name == "airborne_ratio") {
				definedAirborneRatio = true;
				airborneRatio = xml->readElementText().toDouble();
			} else if (tag_name == "phial_ratio") {
				definedPhialRatio = true;
				phialRatio = xml->readElementText().toDouble();
			} else if (tag_name == "minds_eye_ratio") {
				definedMindsEyeRatio = true;
				mindsEyeRatio = xml->readElementText().toDouble();
			} else {
				XML_SKIP_CURRENT_ELEMENT(*xml);
			}
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
}
