#ifndef BuildWithDps_h_
#define BuildWithDps_h_

#include "NamedObject.h"
#include "Damage.h"
#include "Dps.h"

struct Build;
struct Target;
struct Profile;

struct BuildWithDps {
	Build *build;
	Damage damage;
	Dps dps;

	BuildWithDps();
	BuildWithDps(const BuildWithDps &o);
	// Note : takes ownership of build
	explicit BuildWithDps(Build *b);
	BuildWithDps(Build *b, const Profile &profile, const Target &target);
	~BuildWithDps();
	BuildWithDps &operator=(const BuildWithDps &o);
	void compute(const Profile &profile, const Target &target);
};

#endif
