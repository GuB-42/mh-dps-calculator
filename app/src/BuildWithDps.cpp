#include "BuildWithDps.h"

#include <QVector>
#include "BuffWithCondition.h"
#include "Damage.h"
#include "Build.h"
#include "Dps.h"
#include "Profile.h"
#include "Weapon.h"
#include "Item.h"

BuildWithDps::BuildWithDps() :
	build(NULL)
{
}

BuildWithDps::BuildWithDps(Build *b, const Profile &profile, const Target &target) :
	build(b)
{
	if (build->weapon) {
		QVector<const BuffWithCondition *> bwc;
		build->getBuffWithConditions(&bwc);
		foreach(Pattern *pattern, profile.patterns) {
			damage.addPattern(bwc, *build->weapon, *pattern);
		}
	}
	dps.compute(target, damage);
}

BuildWithDps::~BuildWithDps() {
	delete build;
}
