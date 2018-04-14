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

struct PatternWithBuffs {
	PatternWithBuffs(const Pattern *pattern,
	                 const QVector<const BuffWithCondition *> &buff_conds,
	                 double raw_weapon, bool awakened_weapon,
	                 double base_affinity) :
		pattern(pattern),
		foldedBuffs(buff_conds, *pattern->conditionRatios,
		            raw_weapon, awakened_weapon, base_affinity)
	{
	}
	const Pattern *pattern;
	FoldedBuffs foldedBuffs;
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
		foreach(Pattern *pattern, profile.patterns) {
			PatternWithBuffs *pwb =
				new PatternWithBuffs(pattern, bwc,
				                     raw_weapon, build->weapon->awakened,
				                     build->weapon->affinity);
			damage.addSharpnessUse(pwb->foldedBuffs, *build->weapon, *pattern);
			patterns.append(pwb);
		}
		foreach(PatternWithBuffs *pwb, patterns) {
			damage.addPattern(pwb->foldedBuffs, *build->weapon, *pwb->pattern);
			delete pwb;
		}
	}
	dps.compute(target, damage);
}
