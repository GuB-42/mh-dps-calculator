#include "Build.h"

#include <QTextStream>
#include "Item.h"
#include "Weapon.h"
#include "BuffGroup.h"

void Build::addItem(Item *item, bool take_slot) {
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
	usedItems << item;
	foreach(const Item::BuffRef &buff_ref, item->buffRefs) {
		if (buff_ref.buffGroup) {
			QMap<const BuffGroup *, int>::iterator it =
				buffLevels.find(buff_ref.buffGroup);
			if (it == buffLevels.end()) {
				it = buffLevels.insert(buff_ref.buffGroup, 0);
			}
			*it += buff_ref.level;
		}
	}
}

void Build::addWeapon(Weapon *weapon) {
	foreach(int s, weapon->decorationSlots) decorationSlots << s;
}

void Build::getBuffWithConditions(QVector<const BuffWithCondition *> *pout) const {
	for (QMap<const BuffGroup *, int>::const_iterator it = buffLevels.begin();
	     it != buffLevels.end(); ++it) {
		it.key()->levels.count();
		int level = it.value();
		if (level >= it.key()->levels.count()) {
			level = it.key()->levels.count() - 1;
		}
		foreach(BuffWithCondition *bc, it.key()->levels[level]->buffs) {
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
				QMap<const BuffGroup *, int>::const_iterator it =
					buffLevels.find(buff_ref.buffGroup);
				if (it == buffLevels.end() ||
				    it.value() < it.key()->levels.count() - 1) {
					useful_items.append(item);
					break;
				}
			}
		}
	}
	while (!useful_items.isEmpty()) {
		Build *new_build = new Build(*this);
		new_build->addItem(useful_items.last(), true);
		pout->append(new_build);
		new_build->fillSlots(pout, useful_items);
		useful_items.pop_back();
	}
}

void Build::print(QTextStream &stream, QString indent) const {
	stream << indent << "- slots: [";
	for (int i = 0; i < decorationSlots.count(); ++i) {
		if (i > 0) stream << ", ";
		stream << decorationSlots[i];
	}
	stream << "]" << endl;
	foreach(const Item *item, usedItems) {
		stream << indent << "- used item:" << endl;
		item->print(stream, indent + "\t");
	}
	for (QMap<const BuffGroup *, int>::const_iterator it = buffLevels.begin();
	     it != buffLevels.end(); ++it) {
		stream << indent << "- buff group: " << it.value() << endl;
		it.key()->print(stream, indent + "\t");
	}
}
