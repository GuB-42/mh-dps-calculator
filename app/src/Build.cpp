#include "Build.h"

#include <QTextStream>
#include "Item.h"
#include "Weapon.h"
#include "BuffGroup.h"
#include "BuffWithCondition.h"

Build::Build() :
	weapon(NULL), weaponAugmentations(0), weaponSlotUpgrade(0)
{
}

static int find_buff_level(const QVector<BuffWithLevel> &buff_levels,
                           const BuffGroup *group) {
	if (buff_levels.count() <= 0) return -1;
	int bit = buff_levels.count();
	bit |= bit >> 1;
	bit |= bit >> 2;
	bit |= bit >> 4;
	bit |= bit >> 8;
	bit |= bit >> 16;
	bit &= ~bit >> 1;
	int i = 0;
	while (bit) {
		int ni = i | bit;
		if (ni >= buff_levels.count()) {
			;
		} else if (buff_levels[ni].group < group) {
			i = ni;
		} else if (group < buff_levels[ni].group) {
			;
		} else {
			return ni;
		}
		bit >>= 1;
	}
	if (group == buff_levels[i].group) {
		return i;
	} else {
		return -1;
	}
}

int Build::getBuffLevel(const BuffGroup *group) const
{
	int idx = find_buff_level(buffLevels, group);
	return idx != -1 ? buffLevels[idx].level : 0;
}

int Build::addBuffLevel(const BuffGroup *group, int level)
{
	int idx = find_buff_level(buffLevels, group);
	if (idx != -1) {
		int new_level = buffLevels[idx].level + level;
		if (new_level <= 0) {
			buffLevels.remove(idx);
			return 0;
		} else if (new_level > group->levels.count() - 1) {
			new_level = group->levels.count() - 1;
		}
		buffLevels[idx].level = new_level;
		return new_level;
	}
	if (level <= 0) return 0;
	if (level > group->levels.count() - 1) level = group->levels.count() - 1;
	BuffWithLevel new_bl(group, level);
	for (int i = 0; i < buffLevels.count(); ++i) {
		if (new_bl.group < buffLevels[i].group) {
			buffLevels.insert(i, new_bl);
			return level;
		}
	}
	buffLevels.append(new_bl);
	return level;
}

void Build::addItem(const Item *item, bool take_slot) {
	if (take_slot && item->decorationLevel > 0) {
		int best_idx = -1;
		int slot_lvl = -1;
		for (int i = 0; i < decorationSlots.count(); ++i) {
			if (decorationSlots[i] >= item->decorationLevel) {
				if (best_idx < 0 || decorationSlots[i] <= slot_lvl) {
					best_idx = i;
					slot_lvl = decorationSlots[i];
				}
			}
		}
		if (best_idx >= 0) decorationSlots.remove(best_idx);
	}
	foreach(int s, item->decorationSlots) decorationSlots << s;
	if (item->weaponSlotUpgrade) {
		if (weaponSlotUpgrade) {
			for (QVector<int>::iterator it = decorationSlots.begin();
			     it != decorationSlots.end(); ++it) {
				if (*it == weaponSlotUpgrade) {
					*it += item->weaponSlotUpgrade;
					weaponSlotUpgrade += item->weaponSlotUpgrade;
					break;
				}
			}
		} else {
			decorationSlots << item->weaponSlotUpgrade;
			weaponSlotUpgrade = item->weaponSlotUpgrade;
		}
	}
	usedItems << item;
	foreach(const Item::BuffRef &buff_ref, item->buffRefs) {
		if (buff_ref.buffGroup) {
			addBuffLevel(buff_ref.buffGroup, buff_ref.level);
		}
	}
}

void Build::addWeapon(const Weapon *weapon) {
	this->weapon = weapon;
	foreach(int s, weapon->decorationSlots) decorationSlots << s;
	weaponAugmentations += weapon->augmentations;
}

void Build::getBuffWithConditions(QVector<const BuffWithCondition *> *pout) const {
	foreach(const BuffWithLevel &bl, buffLevels) {
		int level = bl.level;
		if (level >= bl.group->levels.count()) {
			level = bl.group->levels.count() - 1;
		}
		foreach(BuffWithCondition *bc, bl.group->levels[level]->buffs) {
			pout->append(bc);
		}
	}
}

void Build::fillSlots(QVector<Build *> *pout, const QVector<Item *> &items) const {
	int max_slot = 0;
	foreach(int s, decorationSlots) {
		if (s > max_slot) max_slot = s;
	}

	QVector<Item *> useful_items;
	foreach(Item *item, items) {
		if (item->decorationLevel && item->decorationLevel <= max_slot) {
			foreach(const Item::BuffRef &buff_ref, item->buffRefs) {
				int level = getBuffLevel(buff_ref.buffGroup);
				if (level < buff_ref.buffGroup->levels.count() - 1) {
					useful_items.append(item);
					break;
				}
			}
		}
	}
	while (!useful_items.isEmpty()) {
		Build *new_build = new Build(*this);
		new_build->usedItems.reserve(new_build->usedItems.count() + 1);
		new_build->addItem(useful_items.last(), true);
		pout->append(new_build);
		new_build->fillSlots(pout, useful_items);
		useful_items.pop_back();
	}
}

void Build::fillWeaponAugmentations(QVector<Build *> *pout, const QVector<Item *> &items) const {
	QVector<Item *> useful_items;
	foreach(Item *item, items) {
		if (item->weaponAugmentationLevel &&
		    item->weaponAugmentationLevel <= weaponAugmentations) {
			if (item->weaponSlotUpgrade > 0) {
				useful_items.append(item);
			} else if (!item->decorationSlots.isEmpty()) {
				useful_items.append(item);
			} else {
				foreach(const Item::BuffRef &buff_ref, item->buffRefs) {
					int level = getBuffLevel(buff_ref.buffGroup);
					if (level < buff_ref.buffGroup->levels.count() - 1) {
						useful_items.append(item);
						break;
					}
				}
			}
		}
	}
	while (!useful_items.isEmpty()) {
		Build *new_build = new Build(*this);
		new_build->usedItems.reserve(new_build->usedItems.count() + 1);
		new_build->addItem(useful_items.last(), false);
		new_build->weaponAugmentations -=
			useful_items.last()->weaponAugmentationLevel;
		pout->append(new_build);
		new_build->fillWeaponAugmentations(pout, useful_items);
		useful_items.pop_back();
	}
}

QVector<Item *> Build::listUsefulItems(const QVector<Item *> &items) const {
	QVector<Item *> useful_items;
	foreach(Item *item, items) {
		if (item->weaponSlotUpgrade) goto item_is_useful;
		if (!item->decorationSlots.isEmpty()) goto item_is_useful;
		foreach(const Item::BuffRef &buff_ref, item->buffRefs) {
			if (!buff_ref.buffGroup) continue;
			foreach(const BuffGroupLevel *bl, buff_ref.buffGroup->levels) {
				if (!bl) continue;
				foreach(const BuffWithCondition *bc, bl->buffs) {
					if (!weapon || bc->isUseful(*weapon)) {
						goto item_is_useful;
					}
				}
			}
		}
		continue;
item_is_useful:
		useful_items.append(item);
	}
	return useful_items;
}

void Build::print(QTextStream &stream, QString indent) const {
	stream << indent << "- slots: [";
	for (int i = 0; i < decorationSlots.count(); ++i) {
		if (i > 0) stream << ", ";
		stream << decorationSlots[i];
	}
	stream << "]" << endl;
	stream << indent <<
		"- weapon augmentations: " << weaponAugmentations << endl;
	stream << indent <<
		"- weapon slot upgrade: " << weaponSlotUpgrade << endl;
	foreach(const Item *item, usedItems) {
		stream << indent << "- used item:" << endl;
		item->print(stream, indent + "\t");
	}
	foreach(const BuffWithLevel &bl, buffLevels) {
		stream << indent << "- buff group: " << bl.level << endl;
		bl.group->print(stream, indent + "\t");
	}
}
