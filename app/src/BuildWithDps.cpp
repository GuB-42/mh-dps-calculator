#include "BuildWithDps.h"

#include <QVector>
#include "BuffWithCondition.h"
#include "Damage.h"
#include "Build.h"
#include "Dps.h"
#include "Profile.h"
#include "Weapon.h"
#include "Item.h"
#include "FoldedBuffs.h"

BuildWithDps::BuildWithDps() : build(NULL) {
}

BuildWithDps::BuildWithDps(const BuildWithDps &o) :
	build(o.build ? new Build(*o.build) : NULL),
	damage(o.damage), dps(o.dps)
{
}

BuildWithDps::BuildWithDps(Build *b) : build(b) {
}

BuildWithDps::BuildWithDps(Build *b, const Profile &profile, const Target &target) :
	build(b)
{
	compute(profile, target);
}

BuildWithDps::~BuildWithDps() {
	delete build;
}

BuildWithDps &BuildWithDps::operator=(const BuildWithDps &o) {
	if (&o == this) return *this;
	delete build;
	build = o.build ? new Build(*o.build) : NULL;
	damage = o.damage;
	dps = o.dps;
	return *this;
}

struct PatternWithBuffs {
	PatternWithBuffs(const Pattern *pattern,
	                 const QVector<const BuffWithCondition *> &buff_conds,
	                 bool raw_weapon, bool awakened_weapon,
	                 double base_affinity) :
		pattern(pattern),
		foldedBuffs(buff_conds, *pattern->conditionRatios,
		            raw_weapon, awakened_weapon, base_affinity),
		rate(1.0)
	{
	}
	const Pattern *pattern;
	FoldedBuffs foldedBuffs;
	double rate;
};

void BuildWithDps::compute(const Profile &profile, const Target &target) {
	damage.sharpenPeriod = profile.sharpenPeriod;
	if (build->weapon) {
		bool raw_weapon = true;
		for (int i = 0; i < ELEMENT_COUNT; ++i) {
			if (build->weapon->elements[i] > 0.0) raw_weapon = false;
		}
		for (int i = 0; i < STATUS_COUNT; ++i) {
			if (build->weapon->statuses[i] > 0.0) raw_weapon = false;
		}

		QVector<const BuffWithCondition *> bwc;
		build->getBuffWithConditions(&bwc);

		QVector<PatternWithBuffs *> patterns;
		double total_usage = 0.0;
		foreach(Pattern *pattern, profile.patterns) {
			double usage_rate = 1.0;
			PatternWithBuffs *pwb =
				new PatternWithBuffs(pattern, bwc,
				                     raw_weapon, build->weapon->awakened,
				                     build->weapon->affinity);

			if (pattern->capacityUpFilter) {
				double v = pwb->foldedBuffs.data[MODE_NORMAL_NORMAL]->
					normalBuffs[BUFF_CAPACITY_UP];
				if (v > 1.0) v = 1.0;
				if (v < 0.0) v = 0.0;
				if (pattern->capacityUpEnabled) {
					usage_rate *= v;
				} else {
					usage_rate *= 1.0 - v;
				}
			}

			foreach(const PatternAmmoRef &pa, pattern->ammoRefs) {
				bool found = false;
				foreach(const WeaponAmmoRef &wa, build->weapon->ammoRefs) {
					if (pa.ammo == wa.ammo) {
						found = true;
						break;
					}
				}
				if (!found) {
					usage_rate = 0.0;
					break;
				}
			}

			if (pattern->usage > 0.0) {
				if (total_usage < 1.0) {
					double pur = usage_rate * pattern->usage;
					if (total_usage + pur <= 1.0) {
						total_usage += pur;
					} else {
						usage_rate *= (1.0 - total_usage) / pur;
						total_usage = 1.0;
					}
				} else {
					usage_rate = 0.0;
				}
			}

			if (usage_rate != 0.0) {
				pwb->rate = usage_rate;
				damage.addSharpnessUse(pwb->foldedBuffs, *build->weapon, *pattern,
				                       pwb->rate);
				patterns.append(pwb);
			} else {
				delete pwb;
			}
		}
		foreach(PatternWithBuffs *pwb, patterns) {
			damage.addPattern(pwb->foldedBuffs, *build->weapon, *pwb->pattern,
			                  pwb->rate);
			delete pwb;
		}
	}
	dps.compute(target, damage);
}
