#ifndef Dps_h_
#define Dps_h_

#include <QString>
#include "enums.h"

class QTextStream;
struct Monster;
struct MonsterHitData;
struct DamageData;
struct Damage;
struct Target;

struct Dps {
	double raw;
	double totalElements;
	double elements[ELEMENT_COUNT];
	double totalStatuses;
	double statuses[STATUS_COUNT];
	double fixed;
	double bounceRate;
	double statusProcRate[STATUS_COUNT];
	double killFrequency;
	double weakSpotRatio;
	double stunRate;

	Dps();
	Dps(const Target &target, const Damage &damage);

	void computeNoStatus(const MonsterHitData &hit_data,
	                     const DamageData &normal_damage,
	                     const DamageData &weak_damage);
	void computeStatus(const Monster &monster,
	                   const DamageData &normal_damage,
	                   const DamageData &weak_damage,
	                   double defense_multiplier);

	void compute(const Target &target, const Damage &damage);

	void combine(const Dps &o, double rate);
	void print(QTextStream &stream, QString indent = QString());
};

#endif
