#include "Weapon.h"

#include <QTextStream>
#include <QXmlStreamReader>

const char *toString(Weapon::PhialType t) {
	switch (t) {
	case Weapon::PHIAL_NONE: return "no_phial";
	case Weapon::PHIAL_ELEMENT: return "element";
	case Weapon::PHIAL_IMPACT: return "impact";
	case Weapon::PHIAL_POWER: return "power";
	default: return "???";
	}
}

const char *toString(Weapon::SharpnessLevel t) {
	switch (t) {
	case Weapon::SHARPNESS_RED: return "red";
	case Weapon::SHARPNESS_ORANGE: return "orange";
	case Weapon::SHARPNESS_YELLOW: return "yellow";
	case Weapon::SHARPNESS_GREEN: return "green";
	case Weapon::SHARPNESS_BLUE: return "blue";
	case Weapon::SHARPNESS_WHITE: return "white";
	case Weapon::SHARPNESS_PURPLE: return "purple";
	default: return "???";
	}
}

Weapon::Weapon() :
	attack(0.0), affinity(0.0), awakened(false), phial(PHIAL_NONE),
	sharpnessPlus(0.0), rare(0)
{
	for (int i = 0; i < ELEMENT_COUNT; ++i) {
		elements[i] = 0.0;
		phialElements[i] = 0.0;
	}
	for (int i = 0; i < STATUS_COUNT; ++i) {
		statuses[i] = 0.0;
		phialStatuses[i] = 0.0;
	}
	for (int i = 0; i < SHARPNESS_COUNT; ++i) {
		sharpness[i] = 0.0;
	}
}

void Weapon::print(QTextStream &stream, QString indent) const {
	NamedObject::print(stream, indent);
	stream << indent << "- type: " << type << endl;
	stream << indent << "- attack: " << attack << endl;
	stream << indent << "- affinity: " << affinity << endl;
	stream << indent << "- awakened: " << (awakened ? "true" : "false") << endl;
	stream << indent << "- elements: [";
	for (int i = 0; i < ELEMENT_COUNT; ++i) {
		if (i > 0) stream << ", ";
		stream << elements[i];
	}
	stream << "]" << endl;
	stream << indent << "- statuses: [";
	for (int i = 0; i < STATUS_COUNT; ++i) {
		if (i > 0) stream << ", ";
		stream << statuses[i];
	}
	stream << "]" << endl;
	stream << indent << "- phial: " << toString(phial) << endl;
	stream << indent << "- phialElements: [";
	for (int i = 0; i < ELEMENT_COUNT; ++i) {
		if (i > 0) stream << ", ";
		stream << phialElements[i];
	}
	stream << "]" << endl;
	stream << indent << "- phialStatuses: [";
	for (int i = 0; i < STATUS_COUNT; ++i) {
		if (i > 0) stream << ", ";
		stream << phialStatuses[i];
	}
	stream << "]" << endl;
	stream << indent << "- sharpness: [";
	for (int i = 0; i < SHARPNESS_COUNT; ++i) {
		if (i > 0) stream << ", ";
		stream << sharpness[i];
	}
	stream << "]" << endl;
	stream << indent << "- sharpnessPlus: " << sharpnessPlus << endl;
	stream << indent << "- slots: [";
	for (int i = 0; i < decorationSlots.count(); ++i) {
		if (i > 0) stream << ", ";
		stream << decorationSlots[i];
	}
	stream << "]" << endl;
	stream << indent << "- rare: " << rare << endl;
}

static void parse_sharpness(QXmlStreamReader *xml,
                            double (*psharpness)[Weapon::SHARPNESS_COUNT],
                            double *pplus) {
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			QStringRef tag_name = xml->name();
			if (tag_name == "plus") {
				*pplus = xml->readElementText().toDouble();
			} else {
				bool found = false;
				for (int i = 0; i < Weapon::SHARPNESS_COUNT; ++i) {
					if (tag_name == toString((Weapon::SharpnessLevel)i)) {
						(*psharpness)[i] = xml->readElementText().toDouble();
						found = true;
						break;
					}
				}
				if (!found) xml->skipCurrentElement();
			}
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
}

static void parse_element_phial(QXmlStreamReader *xml,
                                double (*pelements)[ELEMENT_COUNT],
                                double (*pstatuses)[STATUS_COUNT],
                                Weapon::PhialType *pphial) {
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			QStringRef tag_name = xml->name();
			bool found = false;
			if (pphial) {
				for (int i = 0; i < Weapon::PHIAL_COUNT; ++i) {
					if (tag_name == toString((Weapon::PhialType)i)) {
						*pphial = (Weapon::PhialType)i;
						xml->skipCurrentElement();
						found = true;
						break;
					}
				}
			}
			if (!found) {
				for (int i = 0; i < ELEMENT_COUNT; ++i) {
					if (tag_name == toString((ElementType)i)) {
						(*pelements)[i] = xml->readElementText().toDouble();
						found = true;
						break;
					}
				}
			}
			if (!found) {
				for (int i = 0; i < STATUS_COUNT; ++i) {
					if (tag_name == toString((StatusType)i)) {
						(*pstatuses)[i] = xml->readElementText().toDouble();
						found = true;
						break;
					}
				}
			}
			if (!found) xml->skipCurrentElement();
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
}

static void parse_slots(QXmlStreamReader *xml, QList<int> *dslots) {
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			QStringRef tag_name = xml->name();
			if (tag_name == "slot") {
				dslots->append(xml->readElementText().toInt());
			} else {
				xml->skipCurrentElement();
			}
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
}

void Weapon::readXml(QXmlStreamReader *xml) {
	if (xml->attributes().hasAttribute("id")) {
		id = xml->attributes().value("id").toString();
	}
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			QStringRef tag_name = xml->name();
			if (readXmlName(xml)) {
				; // name
			} else if (tag_name == "type") {
				type = xml->readElementText();
			} else if (tag_name == "attack") {
				attack = xml->readElementText().toDouble();
			} else if (tag_name == "affinity") {
				affinity = xml->readElementText().toDouble();
			} else if (tag_name == "awakened") {
				QString v = xml->readElementText();
				awakened = !(v.isEmpty() || v.toLower() == "false" || v == "0");
			} else if (tag_name == "sharpness") {
				parse_sharpness(xml, &sharpness, &sharpnessPlus);
			} else if (tag_name == "element") {
				parse_element_phial(xml, &elements, &statuses, NULL);
			} else if (tag_name == "phial") {
				parse_element_phial(xml, &phialElements, &phialStatuses, &phial);
			} else if (tag_name == "slots") {
				parse_slots(xml, &decorationSlots);
			} else {
				xml->skipCurrentElement();
			}
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
}
