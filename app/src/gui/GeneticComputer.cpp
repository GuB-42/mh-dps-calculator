#include "GeneticComputer.h"

#include <QtConcurrentMap>
#include <QFutureWatcher>

#include "ResultTableModel.h"
#include "../MainData.h"
#include "../Weapon.h"
#include "../Profile.h"
#include "../Item.h"
#include "../BuildWithDps.h"

GeneticComputer::GeneticComputer(QObject *parent) : QObject(parent) {
}

static bool dps_sort(BuildWithDps *a, BuildWithDps *b) {
	return a->dps.totalDps() > b->dps.totalDps();
}

class ItemSorter {
public:
	bool operator()(const Item *a, const Item *b) const {
		QHash<const Item *, int>::const_iterator ita = sortKey.find(a);
		QHash<const Item *, int>::const_iterator itb = sortKey.find(b);
		if (ita == sortKey.end()) {
			return itb == sortKey.end() ? a < b : false;
		} else {
			return itb == sortKey.end() ? true : *ita < *itb;
		}
	}
	QHash<const Item *, int> sortKey;
};

void GeneticComputer::compute(const Parameters &params) {
	Build *base_build = new Build();
	base_build->buffLevels = params.buffLevels;
	foreach(int s, params.decorationSlots) base_build->addSlot(s);

	ItemSorter item_sort;
	for (int i = 0; i < params.items.count(); ++i) {
		item_sort.sortKey[params.items[i]] = i;
	}

	QVector<const Weapon *> weapons;
	foreach(const Weapon *weapon, params.weapons) {
		if (weapon->type != params.profile->weaponType) continue;
		if (!weapon->final && params.finalOnly) continue;
		weapons.append(weapon);
	}

	QHash<const Weapon *, QVector<const Item *> > useful_items;
	foreach(const Weapon *weapon, weapons) {
		Build tb(*base_build);
		tb.addWeapon(weapon);
		QVector<const Item *> itl;
		foreach(const Item *item, tb.listUsefulItems(params.items, *params.profile)) {
			itl.append(item);
		}
		useful_items[weapon] = itl;
	}

	QVector<BuildWithDps *> bwds;
	bwds.resize(2000);

	bool first = true;
	for (int gen = 0; gen < 100; ++gen) {
		if (!first) {
			for (int i = 0; i < bwds.count(); ++i) {
				int oi = qrand() % bwds.count();
				int oi2 = qrand() % bwds.count();
				if (oi > oi2) {
					int t = oi;
					oi = oi2;
					oi2 = t;
				}
				if (oi < i) {
					*bwds[i] = *bwds[oi];
					if (oi != oi2 && oi2 < i) {
						bwds[i]->build->usedItems <<
						bwds[oi2]->build->usedItems;
					}
				}
			}
		}

		for (int i = 0; i < bwds.count(); ++i) {
			if (i < bwds.count() / 50 && bwds[i]) continue;

			const Weapon *weapon = weapons[qrand() % weapons.count()];
			if (bwds[i] && (qrand() % 100) < 90) {
				weapon = bwds[i]->build->weapon;
			}

			Build *build = new Build(*base_build);
			build->addWeapon(weapon);

			QVector<const Item *> already_items;
			if (bwds[i]) already_items = bwds[i]->build->usedItems;
			QVector<const Item *> new_items = useful_items[weapon];
			while (1) {
				{
					QVector<const Item *> t;
					foreach (const Item *item, already_items) {
						if (build->canAugmentItem(item)) t.append(item);
					}
					already_items = t;
				}
				{
					QVector<const Item *> t;
					foreach (const Item *item, new_items) {
						if (build->canAugmentItem(item)) t.append(item);
					}
					new_items = t;
				}
				if (already_items.isEmpty() && new_items.isEmpty()) break;
				const Item *item;
				if (already_items.isEmpty()) {
					item = new_items[qrand() % new_items.count()];
				} else if (new_items.isEmpty()) {
					item = already_items[qrand() % already_items.count()];
				} else if ((qrand() % 100) < 3) {
					item = new_items[qrand() % new_items.count()];
				} else {
					item = already_items[qrand() % already_items.count()];
				}
				build->weaponAugmentations -=
					item->weaponAugmentationLevel;
				build->addItem(item);
			}

			already_items.clear();
			if (bwds[i]) already_items = bwds[i]->build->usedItems;
			new_items = useful_items[weapon];
			while (1) {
				{
					QVector<const Item *> t;
					foreach (const Item *item, already_items) {
						if (build->canSlotItem(item)) t.append(item);
					}
					already_items = t;
				}
				{
					QVector<const Item *> t;
					foreach (const Item *item, new_items) {
						if (build->canSlotItem(item)) t.append(item);
					}
					new_items = t;
				}
				if (already_items.isEmpty() && new_items.isEmpty()) break;
				const Item *item;
				if (already_items.isEmpty()) {
					item = new_items[qrand() % new_items.count()];
				} else if (new_items.isEmpty()) {
					item = already_items[qrand() % already_items.count()];
				} else if ((qrand() % 100) < 3) {
					item = new_items[qrand() % new_items.count()];
				} else {
					item = already_items[qrand() % already_items.count()];
				}
				build->useSlot(item->decorationLevel);
				build->addItem(item);
			}
			std::sort(build->usedItems.begin(), build->usedItems.end(), item_sort);

			BuildWithDps *bwd = new BuildWithDps(build);
			bwd->compute(*params.profile, *params.target, false);
			delete bwds[i];
			bwds[i] = bwd;
		}

		std::sort(bwds.begin(), bwds.end(), dps_sort);
		first = false;

		printf("gen %d: %g\n", gen, bwds[0]->dps.totalDps());
	}

	emit finished(bwds);
}
