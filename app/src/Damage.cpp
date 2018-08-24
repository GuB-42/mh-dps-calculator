#include "Damage.h"

#include <QTextStream>
#include "DamageData.h"
#include "BuffWithCondition.h"
#include "Weapon.h"
#include "Profile.h"
#include "FoldedBuffs.h"
#include "Constants.h"

Damage::Damage() :
	sharpenPeriod(Constants::instance()->sharpenPeriod)
{
	data[0] = new DamageData();
	isAlias[0] = false;
	for (int i = 1; i < MODE_COUNT; ++i) {
		data[i] = data[0];
		isAlias[i] = true;
	}
	std::fill_n(sharpnessUse, (size_t)MODE_COUNT, 0.0);
}

Damage::Damage(const Damage &o) :
	sharpenPeriod(o.sharpenPeriod)
{
	for (int i = 0; i < MODE_COUNT; ++i) {
		isAlias[i] = o.isAlias[i];
		if (!isAlias[i]) data[i] = new DamageData(*o.data[i]);
	}
	for (int i = 0; i < MODE_COUNT; ++i) {
		if (!isAlias[i]) {
			for (int j = 0; j < MODE_COUNT; ++j) {
				if (isAlias[j] && o.data[i] == o.data[j]) {
					data[j] = data[i];
				}
			}
		}
	}
	std::copy(o.sharpnessUse, o.sharpnessUse + MODE_COUNT, sharpnessUse);
}

Damage::~Damage() {
	for (int i = 0; i < MODE_COUNT; ++i) {
		if (!isAlias[i]) delete data[i];
	}
}

Damage &Damage::operator=(const Damage &o) {
	if (&o == this) return *this;
	sharpenPeriod = o.sharpenPeriod;
	for (int i = 0; i < MODE_COUNT; ++i) {
		if (!isAlias[i]) delete data[i];
		isAlias[i] = o.isAlias[i];
		if (!isAlias[i]) data[i] = new DamageData(*o.data[i]);
	}
	for (int i = 0; i < MODE_COUNT; ++i) {
		if (!isAlias[i]) {
			for (int j = 0; j < MODE_COUNT; ++j) {
				if (isAlias[j] && o.data[i] == o.data[j]) {
					data[j] = data[i];
				}
			}
		}
	}
	std::copy(o.sharpnessUse, o.sharpnessUse + MODE_COUNT, sharpnessUse);
	return *this;
}

void Damage::clear() {
	data[0]->clear();
	for (int i = 1; i < MODE_COUNT; ++i) {
		if (!isAlias[i]) delete data[i];
		data[i] = data[0];
		isAlias[i] = true;
	}
	std::fill_n(sharpnessUse, (size_t)MODE_COUNT, 0.0);
}

void Damage::addSharpnessUse(const FoldedBuffs &folded_buffs,
                             const Weapon &weapon, const Pattern &pattern,
                             double base_rate) {
	double rate = base_rate * pattern.rate / pattern.period;
	for (int mode = 0; mode < MODE_COUNT; ++mode) {
		const FoldedBuffsData *buffs = folded_buffs.data[mode];
		double xaffinity =
			(weapon.affinity + buffs->normalBuffs[BUFF_AFFINITY_PLUS]) / 100.0;
		double sharpness_use = pattern.sharpnessUse * rate *
			buffs->normalBuffs[BUFF_SHARPNESS_USE_MULTIPLIER];
		if (xaffinity > 0.0) {
			sharpness_use *= 1.0 + xaffinity *
				(buffs->normalBuffs[BUFF_SHARPNESS_USE_CRITICAL_MULTIPLIER] - 1.0);
		}
		sharpnessUse[mode] += sharpness_use;
	}
}

void Damage::addPattern(const FoldedBuffs &folded_buffs,
                        const Weapon &weapon, const Pattern &pattern,
                        double base_rate, bool with_details) {
	double rate = base_rate * pattern.rate / pattern.period;

	bool is_set[MODE_COUNT] = { false };
	for (int mode = 0; mode < MODE_COUNT; ++mode) {
		if (!is_set[mode]) {
			DamageData dmg(weapon, *folded_buffs.data[mode],
			               pattern, sharpnessUse[mode] * sharpenPeriod,
			               sharpenPeriod);
			if (with_details) dmg.setBuffs(*folded_buffs.data[mode]);
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
