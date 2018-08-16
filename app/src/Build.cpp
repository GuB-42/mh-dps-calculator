#include "Build.h"

#include <QTextStream>
#include "Item.h"
#include "Weapon.h"
#include "BuffGroup.h"
#include "BuffWithCondition.h"
#include "Song.h"

Build::Build() :
	weapon(NULL), weaponAugmentations(0), weaponSlotUpgrade(0)
{
}

static int find_buff_level(const QVector<BuffWithLevel> &buff_levels,
                           const BuffGroup *group) {
	if (buff_levels.count() <= 0) return -1;
	int bit = buff_levels.count();
	for (int shift = 1; shift <= 16; shift <<= 1) bit |= bit >> shift;
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

int Build::maxBuffLevel(const BuffGroup *group, int level)
{
	int idx = find_buff_level(buffLevels, group);
	if (idx != -1) {
		if (level > buffLevels[idx].level) {
			if (level > group->levels.count() - 1) {
				buffLevels[idx].level = group->levels.count() - 1;
			} else {
				buffLevels[idx].level = level;
			}
		}
		return buffLevels[idx].level;
	} else {
		return addBuffLevel(group, level);
	}
}

void Build::addBuffSetBonusLevel(const BuffSetBonus *buff_set_bonus, int level)
{
	QVector<BuffSetBonusWithLevel>::iterator it = buffSetBonusLevels.begin();
	for (; it != buffSetBonusLevels.end(); ++it) {
		if (it->buffSetBonus == buff_set_bonus) break;
	}
	int last_level = 0;
	int new_level = 0;
	if (it == buffSetBonusLevels.end()) {
		buffSetBonusLevels.append(BuffSetBonusWithLevel(buff_set_bonus, level));
		new_level = level;
	} else {
		last_level = it->level;
		it->level += level;
		new_level = it->level;
	}
	foreach(const BuffSetBonus::Level &bsl, buff_set_bonus->levels) {
		if (bsl.buffGroup) {
			if (bsl.buffSetLevel > last_level &&
			    bsl.buffSetLevel <= new_level) {
				addBuffLevel(bsl.buffGroup, bsl.buffLevel);
			} else if (bsl.buffSetLevel <= last_level &&
			           bsl.buffSetLevel > new_level) {
				addBuffLevel(bsl.buffGroup, -bsl.buffLevel);
			}
		}
	}
}

void Build::addItem(const Item *item) {
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
	foreach(const Item::BuffSetBonusRef &bsbr, item->buffSetBonusRefs) {
		if (bsbr.buffSetBonus) {
			addBuffSetBonusLevel(bsbr.buffSetBonus, bsbr.level);
		}
	}
}

void Build::addWeapon(const Weapon *weapon) {
	this->weapon = weapon;
	foreach(int s, weapon->decorationSlots) decorationSlots << s;
	weaponAugmentations += weapon->augmentations;
	foreach(Song *song, weapon->songs) {
		foreach(const Song::BuffRef &buff_ref, song->buffRefs) {
			if (buff_ref.buffGroup) {
				maxBuffLevel(buff_ref.buffGroup, buff_ref.level);
			}
		}
	}
}

static bool bwc_sort(const BuffWithCondition *a, const BuffWithCondition *b) {
	return *a < *b;
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
	std::sort(pout->begin(), pout->end(), bwc_sort);
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
		new_build->useSlot(useful_items.last()->decorationLevel);
		new_build->addItem(useful_items.last());
		pout->append(new_build);
		new_build->fillSlots(pout, useful_items);
		useful_items.pop_back();
	}
}

bool Build::useSlot(int slot_level) {
	int best_idx = -1;
	int best_idx_level = -1;
	int best_idx_count = 0;
	for (int i = 0; i < decorationSlots.count(); ++i) {
		if (decorationSlots[i] >= slot_level) {
			if (best_idx_count == 0 || decorationSlots[i] < best_idx_level) {
				best_idx = i;
				best_idx_count = 1;
				best_idx_level = decorationSlots[i];
			} else if (decorationSlots[i] == best_idx_level) {
				best_idx = i;
				++best_idx_count;
			}
		}
	}
	if (best_idx >= 0) {
		if (slot_level == weaponSlotUpgrade && best_idx_count <= 1) {
			weaponSlotUpgrade = 0;
		}
		decorationSlots.remove(best_idx);
		return true;
	} else {
		return false;
	}
}

bool Build::limitSlots(QVector<int> limit) {
	bool downgrade = false;
	QHash<int, int> counts;
	foreach (int s, limit) {
		QHash<int, int>::iterator it = counts.find(s);
		if (it == counts.end()) counts[s] = 1; else ++*it;
	}
	int move_i = 0;
	for (int i = 0; i < decorationSlots.count(); ++i) {
		QHash<int, int>::iterator it = counts.find(decorationSlots[i]);
		if (it != counts.end() && *it) {
			--*it;
			decorationSlots[move_i++] = decorationSlots[i];
		} else {
			if (decorationSlots[i] == weaponSlotUpgrade) {
				downgrade = true;
				weaponSlotUpgrade = 0;
			}
		}
	}
	decorationSlots.resize(move_i);
	return downgrade;
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
		new_build->addItem(useful_items.last());
		new_build->weaponAugmentations -=
			useful_items.last()->weaponAugmentationLevel;
		pout->append(new_build);
		new_build->fillWeaponAugmentations(pout, useful_items);
		useful_items.pop_back();
	}
}

bool Build::isBuffUseful(const BuffGroup *group,
                         const Profile &profile) const {
	if (!group) return false;
	foreach(const BuffGroupLevel *bl, group->levels) {
		if (!bl) continue;
		foreach(const BuffWithCondition *bc, bl->buffs) {
			if (!weapon || bc->isUseful(*weapon, profile)) return true;
		}
	}
	return false;
}

QVector<Item *> Build::listUsefulItems(const QVector<Item *> &items,
                                       const Profile &profile) const {
	QVector<Item *> useful_items;
	foreach(Item *item, items) {
		if (item->weaponSlotUpgrade) goto item_is_useful;
		if (!item->decorationSlots.isEmpty()) goto item_is_useful;
		foreach(const Item::BuffRef &buff_ref, item->buffRefs) {
			if (isBuffUseful(buff_ref.buffGroup, profile)) goto item_is_useful;
		}
		foreach(const Item::BuffSetBonusRef &bsr, item->buffSetBonusRefs) {
			if (!bsr.buffSetBonus) continue;
			foreach(const BuffSetBonus::Level &bsl, bsr.buffSetBonus->levels) {
				if (isBuffUseful(bsl.buffGroup, profile)) goto item_is_useful;
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
