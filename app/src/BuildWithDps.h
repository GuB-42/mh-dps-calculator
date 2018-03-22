#ifndef BuildWithDps_h_
#define BuildWithDps_h_

#include "NamedObject.h"
#include "Damage.h"
#include "Dps.h"

class Build;
class Target;
class Profile;

struct BuildWithDps {
	Build *build;
	Damage damage;
	Dps dps;

	BuildWithDps();
	// Note : takes ownership of build
	BuildWithDps(Build *b, const Profile &profile, const Target &target);
	~BuildWithDps();
};

#endif
