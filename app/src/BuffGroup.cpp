#include "BuffGroup.h"

#include "BuffWithCondition.h"
#include <QTextStream>
#include <QXmlStreamReader>

BuffGroupLevel::~BuffGroupLevel() {
	while (!buffs.isEmpty()) {
		delete buffs.takeLast();
	}
}

void BuffGroupLevel::print(QTextStream &stream, QString indent) const {
	NamedObject::print(stream, indent);
	foreach (const BuffWithCondition *buff, buffs) {
		stream << indent << "- buff " << endl;
		buff->print(stream, indent + "\t");
	}
}

static void sub_parse_buff(QXmlStreamReader *xml,
                           QList<BuffWithCondition *> *list,
                           Condition cond,
                           ElementBuff element_buff,
                           StatusBuff status_buff) {
	QStringRef tag_name = xml->name();

	if (status_buff < 0 || status_buff >= STATUS_BUFF_COUNT) {
		if (element_buff == BUFF_ELEMENT_MULTIPLIER) {
			status_buff = BUFF_STATUS_MULTIPLIER;
		} else if (element_buff == BUFF_ELEMENT_PLUS) {
			status_buff = BUFF_STATUS_PLUS;
		}
	}
	if (element_buff >= 0 && element_buff < ELEMENT_BUFF_COUNT) {
		for (int i = 0; i < ELEMENT_COUNT; ++i) {
			if (tag_name == toString((ElementType)i)) {
				BuffWithCondition *bc = new BuffWithCondition();
				bc->condition = cond;
				bc->buffClass = BuffWithCondition::BUFF_CLASS_ELEMENT;
				bc->element.buff = element_buff;
				bc->element.type = (ElementType)i;
				bc->value = xml->readElementText().toDouble();
				list->append(bc);
				return;
			}
		}
	}
	if (status_buff >= 0 && status_buff < STATUS_BUFF_COUNT) {
		for (int i = 0; i < STATUS_COUNT; ++i) {
			if (tag_name == toString((StatusType)i)) {
				BuffWithCondition *bc = new BuffWithCondition();
				bc->condition = cond;
				bc->buffClass = BuffWithCondition::BUFF_CLASS_STATUS;
				bc->status.buff = status_buff;
				bc->status.type = (StatusType)i;
				bc->value = xml->readElementText().toDouble();
				list->append(bc);
				return;
			}
		}
	}
	for (int i = 0; i < CONDITION_COUNT; ++i) {
		if (tag_name == toString((Condition)i)) {
			while (!xml->atEnd()) {
				QXmlStreamReader::TokenType token_type = xml->readNext();
				if (token_type == QXmlStreamReader::StartElement) {
					sub_parse_buff(xml, list, (Condition)i,
					               element_buff, status_buff);
				} else if (token_type == QXmlStreamReader::EndElement) {
					return;
				}
			}
			return;
		}
	}
	for (int i = 0; i < NORMAL_BUFF_COUNT; ++i) {
		if (tag_name == toString((NormalBuff)i)) {
			BuffWithCondition *bc = new BuffWithCondition();
			bc->condition = cond;
			bc->buffClass = BuffWithCondition::BUFF_CLASS_NORMAL;
			bc->normal.buff = (NormalBuff)i;
			bc->value = xml->readElementText().toDouble();
			list->append(bc);
			return;
		}
	}
	for (int i = 0; i < ELEMENT_BUFF_COUNT; ++i) {
		if (tag_name == toString((ElementBuff)i)) {
			while (!xml->atEnd()) {
				QXmlStreamReader::TokenType token_type = xml->readNext();
				if (token_type == QXmlStreamReader::StartElement) {
					sub_parse_buff(xml, list, cond,
					               (ElementBuff)i, (StatusBuff)-1);
				} else if (token_type == QXmlStreamReader::EndElement) {
					return;
				}
			}
			return;
		}
	}
	for (int i = 0; i < STATUS_BUFF_COUNT; ++i) {
		if (tag_name == toString((StatusBuff)i)) {
			while (!xml->atEnd()) {
				QXmlStreamReader::TokenType token_type = xml->readNext();
				if (token_type == QXmlStreamReader::StartElement) {
					sub_parse_buff(xml, list, cond,
					               (ElementBuff)-1, (StatusBuff)i);
				} else if (token_type == QXmlStreamReader::EndElement) {
					return;
				}
			}
			return;
		}
	}
	xml->skipCurrentElement();
}

void BuffGroupLevel::readXml(QXmlStreamReader *xml, int *plevel) {
	if (xml->attributes().hasAttribute("id")) {
		id = xml->attributes().value("id").toString();
	}
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			QStringRef tag_name = xml->name();
			if (readXmlName(xml)) {
				; // name
			} else if (tag_name == "level") {
				if (plevel) *plevel = xml->readElementText().toDouble();
			} else {
				sub_parse_buff(xml, &buffs, CONDITION_ALWAYS,
				               (ElementBuff)-1, (StatusBuff)-1);
			}
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
}

BuffGroup::BuffGroup() {
	levels.append(NULL);
}

BuffGroup::~BuffGroup() {
	while (!alloc_list.isEmpty()) {
		delete alloc_list.takeLast();
	}
}

void BuffGroup::print(QTextStream &stream, QString indent) const {
	NamedObject::print(stream, indent);
	for (int i = 0; i < levels.count(); ++i) {
		stream << indent << "- level " << i << endl;
		if (levels[i]) levels[i]->print(stream, indent + "\t");
	}
}

void BuffGroup::readXml(QXmlStreamReader *xml) {
	if (xml->attributes().hasAttribute("id")) {
		id = xml->attributes().value("id").toString();
	}
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			QStringRef tag_name = xml->name();
			if (readXmlName(xml)) {
				; // name
			} else if (tag_name == "buff") {
				int level = 1;
				BuffGroupLevel *new_lvl = new BuffGroupLevel;
				alloc_list.append(new_lvl);
				new_lvl->readXml(xml, &level);
				while (levels.count() < level) {
					levels.append(levels.last());
				}
				if (level < levels.count()) {
					BuffGroupLevel *last_val = levels[level];
					for (int i = level;
					     i < levels.count() && levels[i] == last_val; ++i) {
						levels[i] = new_lvl;
					}
				} else {
					levels.append(new_lvl);
				}
			} else {
				xml->skipCurrentElement();
			}
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
}
