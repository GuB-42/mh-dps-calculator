#include "MainData.h"
#include "Monster.h"
#include "Weapon.h"
#include "BuffGroup.h"
#include "Profile.h"
#include "Item.h"

#include <QTextStream>
#include <QXmlStreamReader>

MainData::~MainData() {
	while (!monsters.isEmpty()) {
		delete monsters.takeLast();
	}
	while (!weapons.isEmpty()) {
		delete weapons.takeLast();
	}
	while (!buffGroups.isEmpty()) {
		delete buffGroups.takeLast();
	}
	while (!profiles.isEmpty()) {
		delete profiles.takeLast();
	}
	while (!items.isEmpty()) {
		delete items.takeLast();
	}
}

void MainData::print(QTextStream &stream, QString indent) const {
	foreach(const Monster *monster, monsters) {
		stream << indent << "[monster]" << endl;
		monster->print(stream, indent + "\t");
	}
	foreach(const Weapon *weapon, weapons) {
		stream << indent << "[weapon]" << endl;
		weapon->print(stream, indent + "\t");
	}
	foreach(const BuffGroup *buff_group, buffGroups) {
		stream << indent << "[buff group]" << endl;
		buff_group->print(stream, indent + "\t");
	}
	foreach(const Profile *profile, profiles) {
		stream << indent << "[profile]" << endl;
		profile->print(stream, indent + "\t");
	}
	foreach(const Item *item, items) {
		stream << indent << "[item]" << endl;
		item->print(stream, indent + "\t");
	}
}

void MainData::readXml(QXmlStreamReader *xml) {
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			QStringRef tag_name = xml->name();
			if (tag_name == "monster") {
				Monster *monster = new Monster;
				monster->readXml(xml);
				monsters.append(monster);
			} else if (tag_name == "weapon") {
				Weapon *weapon = new Weapon;
				weapon->readXml(xml);
				weapons.append(weapon);
			} else if (tag_name == "buff_group") {
				BuffGroup *buff_group = new BuffGroup;
				buff_group->readXml(xml);
				buffGroups.append(buff_group);
				if (!buff_group->id.isNull()) {
					buffGroupHash[buff_group->id] = buff_group;
				}
			} else if (tag_name == "weapon_profile") {
				Profile *profile = new Profile;
				profile->readXml(xml);
				profiles.append(profile);
			} else if (tag_name == "item") {
				Item *item = new Item;
				item->readXml(xml);
				items.append(item);
				if (!item->id.isNull()) {
					itemHash[item->id] = item;
				}
			} else {
				xml->skipCurrentElement();
			}
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
}

void MainData::matchData() {
	foreach(Item *item, items) {
		for (QVector<Item::BuffRef>::iterator it = item->buffRefs.begin();
		     it != item->buffRefs.end(); ++it) {
			QHash<QString, BuffGroup *>::const_iterator itb =
				buffGroupHash.find(it->id);
			if (itb != buffGroupHash.end()) it->buffGroup = *itb;
		}
	}
}
