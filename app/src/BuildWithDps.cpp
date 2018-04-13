#include "BuildWithDps.h"

#include <QVector>
#include "BuffWithCondition.h"
#include "Damage.h"
#include "Build.h"
#include "Dps.h"
#include "Profile.h"
#include "Weapon.h"
#include "Item.h"

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

void BuildWithDps::compute(const Profile &profile, const Target &target) {
	if (build->weapon) {
		QVector<const BuffWithCondition *> bwc;
		build->getBuffWithConditions(&bwc);
		foreach(Pattern *pattern, profile.patterns) {
			damage.addPattern(bwc, *build->weapon, *pattern);
		}
	}
	dps.compute(target, damage);
}
