#include "MainData.h"
#include "WeaponType.h"
#include "Ammo.h"
#include "Monster.h"
#include "Weapon.h"
#include "BuffGroup.h"
#include "Profile.h"
#include "Item.h"
#include "Target.h"
#include "MotionValue.h"
#include "Song.h"

#include <QTextStream>
#include <QXmlStreamReader>

MainData::~MainData() {
	foreach(WeaponType *weapon_type, weaponTypes) delete weapon_type;
	foreach(Ammo *ammo, ammos) delete ammo;
	foreach(Monster *monster, monsters) delete monster;
	foreach(Weapon *weapon, weapons) delete weapon;
	foreach(BuffGroup *buffGroup, buffGroups) delete buffGroup;
	foreach(BuffSetBonus *buffSetBonus, buffSetBonuses) delete buffSetBonus;
	foreach(Profile *profile, profiles) delete profile;
	foreach(Item *item, items) delete item;
	foreach(Target *target, targets) delete target;
	foreach(MotionValue *motion_value, motionValues) delete motion_value;
}

void MainData::print(QTextStream &stream, QString indent) const {
	foreach(const WeaponType *weapon_type, weaponTypes) {
		stream << indent << "[weapon type]" << endl;
		weapon_type->print(stream, indent + "\t");
	}
	foreach(const Ammo *ammo, ammos) {
		stream << indent << "[ammo]" << endl;
		ammo->print(stream, indent + "\t");
	}
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
	foreach(const BuffSetBonus *buff_set_bonus, buffSetBonuses) {
		stream << indent << "[buff set bonus]" << endl;
		buff_set_bonus->print(stream, indent + "\t");
	}
	foreach(const Profile *profile, profiles) {
		stream << indent << "[profile]" << endl;
		profile->print(stream, indent + "\t");
	}
	foreach(const Item *item, items) {
		stream << indent << "[item]" << endl;
		item->print(stream, indent + "\t");
	}
	foreach(const Target *target, targets) {
		stream << indent << "[target]" << endl;
		target->print(stream, indent + "\t");
	}
	foreach(const MotionValue *motion_value, motionValues) {
		stream << indent << "[motion value]" << endl;
		motion_value->print(stream, indent + "\t");
	}
	foreach(const Song *song, songs) {
		stream << indent << "[song]" << endl;
		song->print(stream, indent + "\t");
	}
}

void MainData::readXml(QXmlStreamReader *xml) {
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			QStringRef tag_name = xml->name();
			if (tag_name == "weapon_type") {
				WeaponType *weapon_type = new WeaponType;
				weapon_type->readXml(xml);
				weaponTypes.append(weapon_type);
				if (!weapon_type->id.isNull()) {
					weaponTypeHash[weapon_type->id] = weapon_type;
				}
			} else if (tag_name == "ammo") {
				Ammo *ammo = new Ammo;
				ammo->readXml(xml);
				ammos.append(ammo);
				if (!ammo->id.isNull()) {
					ammoHash[ammo->id] = ammo;
				}
			} else if (tag_name == "monster") {
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
			} else if (tag_name == "set_bonus") {
				BuffSetBonus *buff_set_bonus = new BuffSetBonus;
				buff_set_bonus->readXml(xml);
				buffSetBonuses.append(buff_set_bonus);
				if (!buff_set_bonus->id.isNull()) {
					buffSetBonusHash[buff_set_bonus->id] = buff_set_bonus;
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
			} else if (tag_name == "target") {
				Target *target = new Target;
				target->readXml(xml);
				targets.append(target);
			} else if (tag_name == "motion_value") {
				MotionValue *motion_value = new MotionValue;
				motion_value->readXml(xml);
				motionValues.append(motion_value);
				if (!motion_value->id.isNull()) {
					motionValueHash[motion_value->id] = motion_value;
				}
			} else if (tag_name == "song") {
				Song *song = new Song;
				song->readXml(xml);
				songs.append(song);
			} else {
				xml->skipCurrentElement();
			}
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
}

void MainData::matchData() {
	foreach(Weapon *weapon, weapons) {
		QHash<QString, WeaponType *>::const_iterator itt =
			weaponTypeHash.find(weapon->weaponTypeRefId);
		if (itt != weaponTypeHash.end()) weapon->type = *itt;
		for (QVector<WeaponAmmoRef>::iterator it = weapon->ammoRefs.begin();
		     it != weapon->ammoRefs.end(); ++it) {
			QHash<QString, Ammo *>::const_iterator ita = ammoHash.find(it->id);
			if (ita != ammoHash.end()) it->ammo = *ita;
		}
	}
	foreach(BuffSetBonus *bsb, buffSetBonuses) {
		for (QVector<BuffSetBonus::Level>::iterator it = bsb->levels.begin();
		     it != bsb->levels.end(); ++it) {
			QHash<QString, BuffGroup *>::const_iterator itb =
				buffGroupHash.find(it->buffId);
			if (itb != buffGroupHash.end()) it->buffGroup = *itb;
		}
	}
	foreach(Item *item, items) {
		for (QVector<Item::BuffRef>::iterator it = item->buffRefs.begin();
		     it != item->buffRefs.end(); ++it) {
			QHash<QString, BuffGroup *>::const_iterator itb =
				buffGroupHash.find(it->id);
			if (itb != buffGroupHash.end()) it->buffGroup = *itb;
		}
		for (QVector<Item::BuffSetBonusRef>::iterator it =
		     item->buffSetBonusRefs.begin();
		     it != item->buffSetBonusRefs.end(); ++it) {
			QHash<QString, BuffSetBonus *>::const_iterator itb =
				buffSetBonusHash.find(it->id);
			if (itb != buffSetBonusHash.end()) it->buffSetBonus = *itb;
		}
	}
	foreach(Song *song, songs) {
		for (QVector<Song::BuffRef>::iterator it = song->buffRefs.begin();
		     it != song->buffRefs.end(); ++it) {
			QHash<QString, BuffGroup *>::const_iterator itb =
				buffGroupHash.find(it->id);
			if (itb != buffGroupHash.end()) it->buffGroup = *itb;
		}
	}
	foreach(Target *target, targets) {
		target->matchMonsters(monsters);
	}
	foreach(MotionValue *mv, motionValues) {
		QHash<QString, WeaponType *>::const_iterator itt =
			weaponTypeHash.find(mv->weaponTypeRefId);
		if (itt != weaponTypeHash.end()) mv->weaponType = *itt;
	}
	foreach(Profile *profile, profiles) {
		QHash<QString, WeaponType *>::const_iterator itt =
			weaponTypeHash.find(profile->weaponTypeRefId);
		if (itt != weaponTypeHash.end()) profile->weaponType = *itt;
		foreach(Pattern *pattern, profile->patterns) {
			pattern->applyMotionValues(motionValueHash);
			for (QVector<PatternAmmoRef>::iterator it = pattern->ammoRefs.begin();
			     it != pattern->ammoRefs.end(); ++it) {
				QHash<QString, Ammo *>::const_iterator ita = ammoHash.find(it->id);
				if (ita != ammoHash.end()) it->ammo = *ita;
			}
		}
	}
	foreach(Weapon *weapon, weapons) {
		if (weapon->notes.isEmpty()) continue;
		foreach(Song *song, songs) {
			bool found = true;
			foreach(Note note, song->notes) {
				if (!weapon->notes.contains(note)) {
					found = false;
					break;
				}
			}
			if (found) weapon->songs.append(song);
		}
	}
}
