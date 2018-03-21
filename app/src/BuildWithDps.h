#ifndef BuildWithDps_h_
#define BuildWithDps_h_

class Build;
class Dps;
class Damage;
class Target;
class Profile;

struct BuildWithDps {
	Build *build;
	Damage *damage;
	Dps *dps;

	BuildWithDps();
	// Note : takes ownership of build
	BuildWithDps(Build *b, const Profile &profile, const Target &target);
	~BuildWithDps();
};

#endif
