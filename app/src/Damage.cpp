#include "Damage.h"

#include <QTextStream>
#include "DamageData.h"
#include "BuffWithCondition.h"
#include "Weapon.h"
#include "Profile.h"
#include "FoldedBuffs.h"

Damage::Damage() {
	data[0] = new DamageData();
	isAlias[0] = false;
	for (int i = 1; i < MODE_COUNT; ++i) {
		data[i] = data[0];
		isAlias[i] = true;
	}
}

Damage::~Damage() {
	for (int i = 0; i < MODE_COUNT; ++i) {
		if (!isAlias[i]) delete data[i];
	}
}

void Damage::addPattern(const QVector<const BuffWithCondition *> &buff_conds,
                        const Weapon &weapon, const Pattern &pattern,
                        double element_status_crit_adjustment) {
	bool raw_weapon = true;
	for (int i = 0; i < ELEMENT_COUNT; ++i) {
		if (weapon.elements[i] > 0.0) raw_weapon = false;
	}
	for (int i = 0; i < STATUS_COUNT; ++i) {
		if (weapon.statuses[i] > 0.0) raw_weapon = false;
	}

	FoldedBuffs folded_buffs(buff_conds, *pattern.conditionRatios,
		                     raw_weapon, weapon.awakened,
		                     weapon.affinity);

	double rate = pattern.rate / pattern.period;

	bool is_set[MODE_COUNT] = { false };
	for (int mode = 0; mode < MODE_COUNT; ++mode) {
		if (!is_set[mode]) {
			DamageData dmg(weapon, *folded_buffs.data[mode], pattern,
			               element_status_crit_adjustment);
			DamageData *old_data = NULL;
			DamageData *new_data = data[mode];
			if (isAlias[mode]) {
				old_data = new_data;
				new_data = new DamageData(*new_data);
				isAlias[mode] = false;
			}
			for (int j = mode + 1; j < MODE_COUNT; ++j) {
				if (folded_buffs.data[mode] == folded_buffs.data[j]) {
					if (data[mode] == data[j]) {
						data[j] = new_data;
					} else {
						if (isAlias[j]) {
							data[j] = new DamageData(*data[j]);
							isAlias[j] = false;
						}
						data[j]->combine(dmg, rate);
					}
					is_set[j] = true;
				} else {
					if (data[j] == new_data) {
						if (old_data) {
							data[j] = old_data;
						} else {
							old_data = new DamageData(*new_data);
							data[j] = old_data;
							isAlias[j] = false;
						}
					}
				}
			}
			new_data->combine(dmg, rate);
			data[mode] = new_data;
			is_set[mode] = true;
		}
	}
}

void Damage::print(QTextStream &stream, QString indent) const {
	for (int i = 0; i < MODE_COUNT; ++i) {
		if (isAlias[i]) continue;
		stream << indent << "- " << toString((MonsterMode)i) << endl;
		for (int j = i + 1; j < MODE_COUNT; ++j) {
			if (data[i] == data[j]) {
				stream << indent << "- " << toString((MonsterMode)j) << endl;
			}
		}
		data[i]->print(stream, indent + "\t");
	}
}
