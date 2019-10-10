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
	if (!buffRefs.isEmpty()) {
		stream << indent << "- buff refs:" << endl;
		foreach(const BuffRef &buff_ref, buffRefs) {
			buff_ref.print(stream, indent + "\t");
		}
	}
	foreach(const BuffSetBonusRef &bsbr, buffSetBonusRefs) {
		stream << indent << "- set bonus ref: " << bsbr.id <<
			"[" << bsbr.level << "]";
		if (bsbr.buffSetBonus) stream << " *";
		stream << endl;
	}
}

static void parse_buff_refs(QXmlStreamReader *xml, QVector<BuffRef> *pout)
{
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			if (xml->name() == "buff_ref") {
				BuffRef br;
				br.readXml(xml);
				pout->append(br);
			} else {
				XML_SKIP_CURRENT_ELEMENT(*xml);
			}
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
}

static void parse_buff_set_bonus_refs(QXmlStreamReader *xml,
                                      QVector<BuffSetBonusRef> *pout)
{
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			if (xml->name() == "set_bonus_ref") {
				BuffSetBonusRef bsbr;
				bsbr.readXml(xml);
				pout->append(bsbr);
			} else {
				XML_SKIP_CURRENT_ELEMENT(*xml);
			}
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

static void parse_categories(QXmlStreamReader *xml, QVector<QString> *pout)
{
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			if (xml->name() == "category_ref") {
				pout->append(xml->attributes().value("id").toString());
			}
			XML_SKIP_CURRENT_ELEMENT(*xml);
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
}

bool Item::readXmlElement(QXmlStreamReader *xml) {
	QStringRef tag_name = xml->name();
	if (NamedObject::readXmlElement(xml)) {
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
	} else if (tag_name == "categories") {
		parse_categories(xml, &categoryRefIds);
	} else {
		return false;
	}
	return true;
}
