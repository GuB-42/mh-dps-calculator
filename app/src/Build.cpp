#include "Build.h"

#include <algorithm>
#include <QTextStream>
#include "QtCompatibility.h"
#include <QHash>
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
	int count = buff_levels.count();
	if (count <= 0) return -1;

	if (count < 16) {
		for (int i = 0; i < count; ++i) {
			if (group == buff_levels[i].group) return i;
		}
		return -1;
	}

	int bit = count;
	bit |= bit >> 1;
	bit |= bit >> 2;
	bit |= bit >> 4;
	bit |= bit >> 8;
	bit |= bit >> 16;
	bit = (bit + 1) >> 1;
	int i = 0;
	while (bit) {
		int ni = i | bit;
		if (ni >= count) {
			;
		} else if (buff_levels[ni].group == group) {
			return ni;
		} else {
			i = group < buff_levels[ni].group ? i : ni;
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
	if (idx == -1) return 0;
	if (buffLevels[idx].levelUncapped ||
	    group->levelCap == BuffGroup::LEVEL_UNCAPPED ||
	    buffLevels[idx].level <= group->levelCap) {
		return buffLevels[idx].level;
	} else {
		return group->levelCap;
	}
}

void Build::updateGetUncap(BuffWithLevel *pbl, const BuffGroup *exclude_group)
{
	if (pbl->group->levelCap == BuffGroup::LEVEL_UNCAPPED) return;
	foreach(const BuffWithLevel &bwl, buffLevels) {
		if (bwl.group == exclude_group) continue;
		const BuffGroupLevel *group_level = bwl.group->levels[bwl.level];
		if (!group_level) continue;
		foreach(const BuffRef &buff_ref, group_level->buffUncaps) {
			if (buff_ref.buffGroup == pbl->group) {
				pbl->levelUncapped = true;
				return;
			}
		}
	}
	pbl->levelUncapped = false;
}

void Build::updateSetUncap(const BuffGroup *group, int old_level, int new_level)
{
	if (old_level == new_level) return;
	const BuffGroupLevel *old_group_level = group->levels[old_level];
	const BuffGroupLevel *new_group_level = group->levels[new_level];
	if (old_group_level == new_group_level) return;
	if (old_group_level) {
		foreach(const BuffRef &old_uncap_ref, old_group_level->buffUncaps) {
			int idx;
			if (!old_uncap_ref.buffGroup) continue;
			if (new_group_level) {
				foreach(const BuffRef &new_uncap_ref, new_group_level->buffUncaps) {
					if (old_uncap_ref.buffGroup == new_uncap_ref.buffGroup) {
						goto updateSetUncap_end;
					}
				}
			}
			idx = find_buff_level(buffLevels, old_uncap_ref.buffGroup);
			if (idx != -1) updateGetUncap(&buffLevels[idx], group);
updateSetUncap_end:;
		}
	} else {
		foreach(const BuffRef &new_uncap_ref, new_group_level->buffUncaps) {
			if (!new_uncap_ref.buffGroup) continue;
			int idx = find_buff_level(buffLevels, new_uncap_ref.buffGroup);
			if (idx != -1) buffLevels[idx].levelUncapped = true;
		}
	}
}

int Build::addBuffLevel(const BuffGroup *group, int level)
{
	int idx = find_buff_level(buffLevels, group);
	if (idx != -1) {
		int new_level = buffLevels[idx].level + level;
		if (new_level <= 0) {
			updateSetUncap(group, buffLevels[idx].level, 0);
			buffLevels.remove(idx);
			return 0;
		} else if (new_level > group->levels.count() - 1) {
			new_level = group->levels.count() - 1;
		}
		updateSetUncap(group, buffLevels[idx].level, new_level);
		buffLevels[idx].level = new_level;
		return new_level;
	}
	if (level <= 0) return 0;
	if (level > group->levels.count() - 1) level = group->levels.count() - 1;
	BuffWithLevel new_bl(group, level);
	updateGetUncap(&new_bl);
	updateSetUncap(group, 0, level);
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
			level = group->levels.count() - 1;
		}
		if (level != buffLevels[idx].level) {
			updateSetUncap(group, buffLevels[idx].level, level);
			buffLevels[idx].level = level;
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
		if (bsl.buffRef.buffGroup) {
			if (bsl.buffSetLevel > last_level &&
			    bsl.buffSetLevel <= new_level) {
				addBuffLevel(bsl.buffRef.buffGroup, bsl.buffRef.level);
			} else if (bsl.buffSetLevel <= last_level &&
			           bsl.buffSetLevel > new_level) {
				addBuffLevel(bsl.buffRef.buffGroup, -bsl.buffRef.level);
			}
		}
	}
}

int Build::getNbSlots() const
{
	int ret = 0;
	for (QVector<SlotWithCount>::const_iterator
	     it = decorationSlots.begin(); it != decorationSlots.end(); ++it) {
		ret += it->count;
	}
	return ret;
}

void Build::addSlot(int slot_level)
{
	for (QVector<SlotWithCount>::iterator
	     it = decorationSlots.begin(); it != decorationSlots.end(); ++it) {
		if (it->level == slot_level) {
			++it->count;
			return;
		} else if (it->level < slot_level) {
			decorationSlots.insert(it, SlotWithCount(slot_level, 1));
			return;
		}
	}
	decorationSlots.append(SlotWithCount(slot_level, 1));
}

void Build::addItem(const Item *item) {
	foreach(int s, item->decorationSlots) addSlot(s);
	if (item->weaponSlotUpgrade) {
		if (weaponSlotUpgrade) {
			useSlot(weaponSlotUpgrade);
			weaponSlotUpgrade += item->weaponSlotUpgrade;
			addSlot(weaponSlotUpgrade);
		} else {
			addSlot(item->weaponSlotUpgrade);
			weaponSlotUpgrade = item->weaponSlotUpgrade;
		}
	}
	if (item != weapon) usedItems << item;
	foreach(const BuffRef &buff_ref, item->buffRefs) {
		if (buff_ref.buffGroup) {
			addBuffLevel(buff_ref.buffGroup, buff_ref.level);
		}
	}
	foreach(const BuffSetBonusRef &bsbr, item->buffSetBonusRefs) {
		if (bsbr.buffSetBonus) {
			addBuffSetBonusLevel(bsbr.buffSetBonus, bsbr.level);
		}
	}
}

void Build::addWeapon(const Weapon *weapon) {
	this->weapon = weapon;
	weaponAugmentations += weapon->augmentations;
	foreach(Song *song, weapon->songs) {
		foreach(const BuffRef &buff_ref, song->buffRefs) {
			if (buff_ref.buffGroup) {
				maxBuffLevel(buff_ref.buffGroup, buff_ref.level);
			}
		}
	}
	addItem(weapon);
}

static bool bwc_sort(const BuffWithCondition *a, const BuffWithCondition *b) {
	return *a < *b;
}

void Build::getBuffWithConditions(QVector<const BuffWithCondition *> *pout) const {
	foreach(const BuffWithLevel &bl, buffLevels) {
		int level = bl.level;
		if (bl.group->levelCap != BuffGroup::LEVEL_UNCAPPED &&
		    !bl.levelUncapped && level > bl.group->levelCap) {
			level = bl.group->levelCap;
		}
		foreach(BuffWithCondition *bc, bl.group->levels[level]->buffs) {
			pout->append(bc);
		}
	}
	std::sort(pout->begin(), pout->end(), bwc_sort);
}

void Build::fillSlots(QVector<Build *> *pout, const QVector<Item *> &items) const {
	QVector<Item *> useful_items;
	foreach(Item *item, items) {
		if (canSlotItem(item)) useful_items.append(item);
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
	QVector<SlotWithCount>::iterator last_it = decorationSlots.end();
	for (QVector<SlotWithCount>::iterator
	     it = decorationSlots.begin(); it != decorationSlots.end(); ++it) {
		if (it->level == slot_level) {
			--it->count;
			if (it->count <= 0) decorationSlots.erase(it);
			return true;
		} else if (it->level < slot_level) {
			break;
		}
		last_it = it;
	}
	if (last_it != decorationSlots.end()) {
		--last_it->count;
		if (last_it->count <= 0) decorationSlots.erase(last_it);
		return true;
	}
	return false;
}

bool Build::limitSlots(QVector<int> limit) {
	bool downgrade = false;
	QHash<int, int> counts;
	foreach (int s, limit) {
		QHash<int, int>::iterator it = counts.find(s);
		if (it == counts.end()) counts[s] = 1; else ++*it;
	}
	QVector<SlotWithCount>::iterator it = decorationSlots.begin();
	while (it != decorationSlots.end()) {
		QHash<int, int>::iterator itc = counts.find(it->level);
		if (itc == counts.end()) {
			if (it->level == weaponSlotUpgrade) {
				downgrade = true;
				weaponSlotUpgrade = 0;
			}
			it = decorationSlots.erase(it);
		} else {
			if (it->count > itc.value()) it->count = itc.value();
			++it;
		}
	}
	return downgrade;
}

void Build::fillWeaponAugmentations(QVector<Build *> *pout, const QVector<Item *> &items) const {
	QVector<Item *> useful_items;
	foreach(Item *item, items) {
		if (canAugmentItem(item)) useful_items.append(item);
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
		foreach(const BuffRef &ref, bl->buffUncaps) {
			if (!ref.buffGroup) continue;
			if (isBuffUseful(ref.buffGroup, profile)) return true;
		}
	}
	return false;
}

bool Build::canSlotItem(const Item *item) const {
	if (!item->decorationLevel) return false;
	if (decorationSlots.isEmpty()) return false;
	if (item->decorationLevel > decorationSlots[0].level) return false;
	for (QVector<BuffRef>::const_iterator it = item->buffRefs.begin();
	     it != item->buffRefs.end(); ++it) {
		int level = getBuffLevel(it->buffGroup);
		if (level < it->buffGroup->levels.count() - 1) return true;
	}
	return false;
}

bool Build::canAugmentItem(const Item *item) const {
	if (!item->weaponAugmentationLevel) return false;
	if (item->weaponAugmentationLevel > weaponAugmentations) return false;
	if (item->weaponSlotUpgrade > 0) return true;
	if (!item->decorationSlots.isEmpty()) return true;
	for (QVector<BuffRef>::const_iterator it = item->buffRefs.begin();
	     it != item->buffRefs.end(); ++it) {
		int level = getBuffLevel(it->buffGroup);
		if (level < it->buffGroup->levels.count() - 1) return true;
	}
	return false;
}

QVector<Item *> Build::listUsefulItems(const QVector<Item *> &items,
                                       const Profile &profile) const {
	QVector<Item *> useful_items;
	foreach(Item *item, items) {
		if (item->weaponSlotUpgrade) goto item_is_useful;
		if (!item->decorationSlots.isEmpty()) goto item_is_useful;
		foreach(const BuffRef &buff_ref, item->buffRefs) {
			if (isBuffUseful(buff_ref.buffGroup, profile)) {
				goto item_is_useful;
			}
		}
		foreach(const BuffSetBonusRef &bsr, item->buffSetBonusRefs) {
			if (!bsr.buffSetBonus) continue;
			foreach(const BuffSetBonus::Level &bsl, bsr.buffSetBonus->levels) {
				if (isBuffUseful(bsl.buffRef.buffGroup, profile)) {
					goto item_is_useful;
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
	bool first = true;
	foreach(const SlotWithCount &swc, decorationSlots) {
		for (int i = 0; i < swc.count; ++i) {
			if (first) {
				first = false;
			} else {
				stream << ", ";
			}
			stream << swc.level;
		}
	}
	stream << "]" << ENDL;
	stream << indent <<
		"- weapon augmentations: " << weaponAugmentations << ENDL;
	stream << indent <<
		"- weapon slot upgrade: " << weaponSlotUpgrade << ENDL;
	foreach(const Item *item, usedItems) {
		stream << indent << "- used item:" << ENDL;
		item->print(stream, indent + "\t");
	}
	foreach(const BuffWithLevel &bl, buffLevels) {
		stream << indent << "- buff group: " << bl.level;
		if (bl.group->levelCap != BuffGroup::LEVEL_UNCAPPED) {
			if (bl.levelUncapped) {
				stream << " (uncapped)";
			} else {
				stream << " (cap: " << bl.group->levelCap << ")";
			}
		}
		stream << ENDL;
		bl.group->print(stream, indent + "\t");
	}
}
