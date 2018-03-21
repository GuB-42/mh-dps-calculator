#include "BuildWithDps.h"

#include <QVector>
#include "BuffWithCondition.h"
#include "Damage.h"
#include "Build.h"
#include "Dps.h"
#include "Profile.h"

BuildWithDps::BuildWithDps() :
	build(NULL), damage(NULL), dps(NULL)
{
}

BuildWithDps::BuildWithDps(Build *b, const Profile &profile, const Target &target) :
	build(b), damage(new Damage())
{
	if (build->weapon) {
		QVector<const BuffWithCondition *> bwc;
		build->getBuffWithConditions(&bwc);
		foreach(Pattern *pattern, profile.patterns) {
			damage->addPattern(bwc, *build->weapon, *pattern);
		}
	}
	dps = new Dps(target, *damage);
}

BuildWithDps::~BuildWithDps() {
	delete build;
	delete damage;
	delete dps;
}
