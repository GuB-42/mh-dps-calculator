#ifndef Build_h_
#define Build_h_

#include <QVector>
#include <QString>

class QTextStream;
struct BuffGroup;
struct BuffSetBonus;
struct Item;
struct BuffWithCondition;
struct Weapon;
struct Profile;

struct BuffWithLevel {
	BuffWithLevel() : group(NULL), level(0), levelUncapped(false) { }
	BuffWithLevel(const BuffGroup *g, int l) :
		group(g), level(l), levelUncapped(false) { }
	const BuffGroup *group;
	int level;
	bool levelUncapped;
	bool operator<(const BuffWithLevel& o) const {
		if (group < o.group) return true;
		if (o.group < group) return false;
		return level < o.level;
    }
};

struct BuffSetBonusWithLevel {
	BuffSetBonusWithLevel() : buffSetBonus(NULL), level(0) { }
	BuffSetBonusWithLevel(const BuffSetBonus *bs, int l) :
		buffSetBonus(bs), level(l) { }
	const BuffSetBonus *buffSetBonus;
	int level;
};

struct Build {
	struct SlotWithCount {
		SlotWithCount() : level(0), count(0) {}
		SlotWithCount(int l, int c) : level(l), count(c) {}
		int level;
		int count;
	};

	const Weapon *weapon;
	QVector<SlotWithCount> decorationSlots;
	int weaponAugmentations;
	int weaponSlotUpgrade;
	QVector<BuffWithLevel> buffLevels;
	QVector<BuffSetBonusWithLevel> buffSetBonusLevels;
	QVector<const Item *> usedItems;

	Build();
	int getBuffLevel(const BuffGroup *group) const;
	int addBuffLevel(const BuffGroup *group, int level);
	int maxBuffLevel(const BuffGroup *group, int level);
	void addBuffSetBonusLevel(const BuffSetBonus *buff_set_bonus, int level);
	int getNbSlots() const;
	void addSlot(int slot_level);
	void addItem(const Item *item);
	void addWeapon(const Weapon *weapon);
	void getBuffWithConditions(QVector<const BuffWithCondition *> *pout) const;
	void fillSlots(QVector<Build *> *pout, const QVector<Item *> &items) const;
	bool useSlot(int slot_level);
	bool limitSlots(QVector<int> limit);
	void fillWeaponAugmentations(QVector<Build *> *pout, const QVector<Item *> &items) const;
	bool isBuffUseful(const BuffGroup *group, const Profile &profile) const;
	bool canSlotItem(const Item *item) const;
	bool canAugmentItem(const Item *item) const;
	QVector<Item *> listUsefulItems(const QVector<Item *> &items, const Profile &profile) const;
	void print(QTextStream &stream, QString indent = QString()) const;

private:
	void updateGetUncap(BuffWithLevel *pbl, const BuffGroup *exclude_group = NULL);
	void updateSetUncap(const BuffGroup *group, int old_level, int new_level);
};

#endif
