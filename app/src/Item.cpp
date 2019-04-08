#include "Item.h"

#include <QTextStream>
#include <QXmlStreamReader>
#include "QtCompatibility.h"

Item::Item() :
	decorationLevel(0), weaponAugmentationLevel(0),
	weaponSlotUpgrade(0), rare(0)
{
}

void Item::print(QTextStream &stream, QString indent) const {
	NamedObject::print(stream, indent);
	stream << indent << "- type: " << type << endl;
	stream << indent << "- decoration level: " << decorationLevel << endl;
	stream << indent << "- rare: " << rare << endl;
	foreach(const BuffRef &buff_ref, buffRefs) {
		stream << indent << "- buff ref: " << buff_ref.id <<
			"[" << buff_ref.level << "]";
		if (buff_ref.buffGroup) stream << " *";
		stream << endl;
	}
	foreach(const BuffSetBonusRef &bsbr, buffSetBonusRefs) {
		stream << indent << "- set bonus ref: " << bsbr.id <<
			"[" << bsbr.level << "]";
		if (bsbr.buffSetBonus) stream << " *";
		stream << endl;
	}
}

static void parse_buff_refs(QXmlStreamReader *xml, QVector<Item::BuffRef> *pout)
{
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			if (xml->name() == "buff_ref") {
				Item::BuffRef br;
				br.id = xml->attributes().value("id").toString();
				if (xml->attributes().hasAttribute("level")) {
					br.level = xml->attributes().value("level").toString().toInt();
				}
				pout->append(br);
			}
			XML_SKIP_CURRENT_ELEMENT(*xml);
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
}

static void parse_buff_set_bonus_refs(QXmlStreamReader *xml,
                                      QVector<Item::BuffSetBonusRef> *pout)
{
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			if (xml->name() == "set_bonus_ref") {
				Item::BuffSetBonusRef bsbr;
				bsbr.id = xml->attributes().value("id").toString();
				if (xml->attributes().hasAttribute("level")) {
					bsbr.level = xml->attributes().value("level").toString().toInt();
				}
				pout->append(bsbr);
			}
			XML_SKIP_CURRENT_ELEMENT(*xml);
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
				XML_SKIP_CURRENT_ELEMENT(*xml);
			}
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
}

void Item::readXml(QXmlStreamReader *xml) {
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
			} else if (tag_name == "slots") {
				parse_slots(xml, &decorationSlots);
			} else if (tag_name == "decoration_level") {
				decorationLevel = xml->readElementText().toInt();
			} else if (tag_name == "weapon_augmentation_level") {
				weaponAugmentationLevel = xml->readElementText().toInt();
			} else if (tag_name == "weapon_slot_upgrade") {
				weaponSlotUpgrade = xml->readElementText().toInt();
			} else if (tag_name == "rare") {
				rare = xml->readElementText().toInt();
			} else if (tag_name == "buff_refs") {
				parse_buff_refs(xml, &buffRefs);
			} else if (tag_name == "set_bonus_refs") {
				parse_buff_set_bonus_refs(xml, &buffSetBonusRefs);
			} else {
				XML_SKIP_CURRENT_ELEMENT(*xml);
			}
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
}
