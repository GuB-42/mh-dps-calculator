#include "Weapon.h"

#include <QTextStream>
#include <QXmlStreamReader>
#include <QHash>
#include "Constants.h"
#include "WeaponType.h"
#include "Ammo.h"

Weapon::Weapon() :
	type(NULL),
	attack(0.0), affinity(0.0), awakened(false), phial(PHIAL_NONE),
	sharpnessPlus(0.0), final(false), rare(0),
	augmentations(0)
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
	stream << indent << "- type: ";
	if (type) {
		stream << type->id << endl;
	} else {
		stream << "<null, " << weaponTypeRefId << ">" << endl;
	}
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
	stream << indent << "- notes: [";
	for (int i = 0; i < notes.count(); ++i) {
		if (i > 0) stream << ", ";
		stream << toString(notes[i]);
	}
	stream << "]" << endl;
	stream << indent << "- ammos: [";
	for (int i = 0; i < ammoRefs.count(); ++i) {
		if (i > 0) stream << ", ";
		if (ammoRefs[i].ammo) {
			stream << ammoRefs[i].ammo->id;
		} else {
			stream << "<null, " << ammoRefs[i].id << ">";
		}
	}
	stream << "]" << endl;
	stream << indent << "- slots: [";
	for (int i = 0; i < decorationSlots.count(); ++i) {
		if (i > 0) stream << ", ";
		stream << decorationSlots[i];
	}
	stream << "]" << endl;
	stream << indent << "- final: " << (final ? "true" : "false") << endl;
	stream << indent << "- rare: " << rare << endl;
}

static void parse_sharpness(QXmlStreamReader *xml,
                            double (*psharpness)[SHARPNESS_COUNT],
                            double *pplus) {
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			QStringRef tag_name = xml->name();
			if (tag_name == "plus") {
				*pplus = xml->readElementText().toDouble();
			} else {
				bool found = false;
				for (int i = 0; i < SHARPNESS_COUNT; ++i) {
					if (tag_name == toString((SharpnessLevel)i)) {
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
                                PhialType *pphial) {
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			QStringRef tag_name = xml->name();
			bool found = false;
			if (pphial) {
				for (int i = 0; i < PHIAL_COUNT; ++i) {
					if (tag_name == toString((PhialType)i)) {
						*pphial = (PhialType)i;
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

static void parse_ammos(QXmlStreamReader *xml, QVector<WeaponAmmoRef> *dammo_refs) {
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			if (xml->name() == "ammo_ref") {
				WeaponAmmoRef ammo_ref;
				ammo_ref.id = xml->attributes().value("id").toString();
				dammo_refs->append(ammo_ref);
			}
			xml->skipCurrentElement();
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
}

static void parse_notes(QXmlStreamReader *xml, QVector<Note> *dnotes) {
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			QStringRef tag_name = xml->name();
			for (int i = 0; i < NOTE_COUNT; ++i) {
				if (tag_name == toString((Note)i)) dnotes->append((Note)i);
			}
			xml->skipCurrentElement();
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
}

static void parse_slots(QXmlStreamReader *xml, QVector<int> *dslots) {
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
			} else if (tag_name == "weapon_type_ref") {
				weaponTypeRefId = xml->attributes().value("id").toString();
				xml->skipCurrentElement();
			} else if (tag_name == "attack") {
				attack = xml->readElementText().toDouble();
			} else if (tag_name == "affinity") {
				affinity = xml->readElementText().toDouble();
			} else if (tag_name == "awakened") {
				QString v = xml->readElementText();
				awakened = !(v.isEmpty() || v.toLower() == "false" || v == "0");
			} else if (tag_name == "final") {
				QString v = xml->readElementText();
				final = !(v.isEmpty() || v.toLower() == "false" || v == "0");
			} else if (tag_name == "rare") {
				rare = xml->readElementText().toInt();
			} else if (tag_name == "sharpness") {
				parse_sharpness(xml, &sharpness, &sharpnessPlus);
			} else if (tag_name == "element") {
				parse_element_phial(xml, &elements, &statuses, NULL);
			} else if (tag_name == "phial") {
				parse_element_phial(xml, &phialElements, &phialStatuses, &phial);
			} else if (tag_name == "ammos") {
				parse_ammos(xml, &ammoRefs);
			} else if (tag_name == "notes") {
				parse_notes(xml, &notes);
			} else if (tag_name == "slots") {
				parse_slots(xml, &decorationSlots);
			} else {
				xml->skipCurrentElement();
			}
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
	if (final && rare) {
		augmentations = 9 - rare;
		if (augmentations < 0) augmentations = 0;
		if (augmentations > 3) augmentations = 3;
	}
}
