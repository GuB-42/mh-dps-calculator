#include "BuffGroup.h"

#include "BuffWithCondition.h"
#include <QTextStream>
#include <QXmlStreamReader>
#include "QtCompatibility.h"

BuffGroupLevel::~BuffGroupLevel() {
	foreach(BuffWithCondition *b, buffs) delete b;
}

void BuffGroupLevel::print(QTextStream &stream, QString indent) const {
	NamedObject::print(stream, indent);
	foreach (const BuffWithCondition *buff, buffs) {
		stream << indent << "- buff " << ENDL;
		buff->print(stream, indent + "\t");
	}
	if (!buffUncaps.isEmpty()) {
		stream << indent << "- buff uncap" << ENDL;
		foreach (const BuffRef &buff_uncap, buffUncaps) {
			buff_uncap.print(stream, indent + "\t");
		}
	}
}

static bool sub_parse_buff(QXmlStreamReader *xml,
                           QVector<BuffWithCondition *> *list,
                           Condition cond,
                           ElementBuff element_buff,
                           StatusBuff status_buff);

static void sub_parse_buff_rec(QXmlStreamReader *xml,
                               QVector<BuffWithCondition *> *list,
                               Condition cond,
                               ElementBuff element_buff,
                               StatusBuff status_buff) {
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			if (!sub_parse_buff(xml, list, cond, element_buff, status_buff)) {
				XML_SKIP_CURRENT_ELEMENT(*xml);
			}
		} else if (token_type == QXmlStreamReader::EndElement) {
			return;
		}
	}
}

static bool sub_parse_buff(QXmlStreamReader *xml,
                           QVector<BuffWithCondition *> *list,
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
				return true;
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
				return true;
			}
		}
	}
	for (int i = 0; i < CONDITION_COUNT; ++i) {
		if (tag_name == toString((Condition)i)) {
			sub_parse_buff_rec(xml, list, (Condition)i,
			                   element_buff, status_buff);
			return true;
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
			return true;
		}
	}
	for (int i = 0; i < ELEMENT_BUFF_COUNT; ++i) {
		if (tag_name == toString((ElementBuff)i)) {
			sub_parse_buff_rec(xml, list, cond,
			                   (ElementBuff)i, (StatusBuff)-1);
			return true;
		}
	}
	for (int i = 0; i < STATUS_BUFF_COUNT; ++i) {
		if (tag_name == toString((StatusBuff)i)) {
			sub_parse_buff_rec(xml, list, cond,
			                   (ElementBuff)-1, (StatusBuff)i);
			return true;
		}
	}
	return false;
}

static void parse_uncap(QXmlStreamReader *xml, QVector<BuffRef> *pout)
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

bool BuffGroupLevel::readXmlElement(QXmlStreamReader *xml) {
	if (NamedObject::readXmlElement(xml)) return true;
	QStringRef tag_name = xml->name();
	if (tag_name == "uncap_buff_level") {
		parse_uncap(xml, &buffUncaps);
		return true;
	} else {
		return sub_parse_buff(xml, &buffs, CONDITION_ALWAYS,
		                      (ElementBuff)-1, (StatusBuff)-1);
	}
}

void BuffGroupLevel::readXmlGetLevel(QXmlStreamReader *xml, int *plevel) {
	if (plevel) {
		if (xml->attributes().hasAttribute("level")) {
			*plevel = xml->attributes().value("level").toString().toInt();
		} else {
			*plevel = 1;
		}
	}
	readXml(xml);
}

BuffGroup::BuffGroup() : levelCap(LEVEL_UNCAPPED) {
	levels.append(NULL);
}

BuffGroup::~BuffGroup() {
	foreach(BuffGroupLevel *v, alloc_list) delete v;
}

bool BuffGroup::hasBuffs() {
	foreach(BuffGroupLevel *level, levels) {
		if (!level) continue;
	    if (!level->buffs.isEmpty()) return true;
		foreach (const BuffRef &buff_ref, level->buffUncaps) {
			BuffGroup *subgroup = buff_ref.buffGroup;
			if (subgroup && subgroup->levelCap != BuffGroup::LEVEL_UNCAPPED) {
				for (int sublevel = subgroup->levelCap;
				     sublevel < subgroup->levels.count(); ++sublevel) {
					if (!subgroup->levels[sublevel]->buffs.isEmpty()) {
						return true;
					}
				}
			}
		}
	}
	return false;
}

void BuffGroup::print(QTextStream &stream, QString indent) const {
	NamedObject::print(stream, indent);
	if (levelCap != LEVEL_UNCAPPED) {
		stream << indent << "- level cap: " << levelCap << ENDL;
	}
	for (int i = 0; i < levels.count(); ++i) {
		stream << indent << "- level " << i << ENDL;
		if (levels[i]) levels[i]->print(stream, indent + "\t");
	}
}

bool BuffGroup::readXmlElement(QXmlStreamReader *xml) {
	if (NamedObject::readXmlElement(xml)) return true;
	QStringRef tag_name = xml->name();
	if (tag_name == "buff") {
		int level = 1;
		BuffGroupLevel *new_lvl = new BuffGroupLevel;
		alloc_list.append(new_lvl);
		new_lvl->readXmlGetLevel(xml, &level);
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
	} else if (tag_name == "level_cap") {
		levelCap = xml->readElementText().toInt();
	} else {
		return false;
	}
	return true;
}

void BuffGroup::readXml(QXmlStreamReader *xml) {
	NamedObject::readXml(xml);

	QHash<BuffGroupLevel *, int> levels_hash;
	for (int i = 0; i < levels.count(); ++i) {
		if (levels[i] && !levels_hash.contains(levels[i])) {
			levels_hash[levels[i]] = i;
		}
	}
	bool single_buff = (levels_hash.size() == 1);
	for (QHash<BuffGroupLevel *, int>::const_iterator it = levels_hash.begin();
	     it != levels_hash.end(); ++it) {
		for (int i = 0; i < LANG_COUNT; ++i) {
			if (!names[i].isNull() && it.key()->names[i].isNull()) {
				if (single_buff) {
					it.key()->names[i] = names[i];
				} else {
					it.key()->names[i] = QString("%1 [%2]").
						arg(names[i]).arg(it.value());
				}
			}
		}
	}
}

void BuffSetBonus::print(QTextStream &stream, QString indent) const {
	NamedObject::print(stream, indent);
	foreach (const Level &level, levels) {
		stream << indent << "- level " << level.buffSetLevel << ENDL;
		level.buffRef.print(stream, indent + "\t");
	}
}

static void parse_buff_set_level(QXmlStreamReader *xml,
                                 QVector<BuffSetBonus::Level> *pout,
                                 int set_level)
{
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			if (xml->name() == "buff_ref") {
				BuffSetBonus::Level lvl;
				lvl.buffSetLevel = set_level;
				lvl.buffRef.readXml(xml);
				pout->append(lvl);
			} else {
				XML_SKIP_CURRENT_ELEMENT(*xml);
			}
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
}

bool BuffSetBonus::readXmlElement(QXmlStreamReader *xml) {
	if (NamedObject::readXmlElement(xml)) return true;
	QStringRef tag_name = xml->name();
	if (tag_name == "set_bonus_level") {
		if (xml->attributes().hasAttribute("level")) {
			parse_buff_set_level(xml, &levels,
			                     xml->attributes().value("level").
			                     toString().toInt());
		} else {
			parse_buff_set_level(xml, &levels, 1);
		}
	} else {
		return false;
	}
	return true;
}
