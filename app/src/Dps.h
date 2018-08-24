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
	double elements[ELEMENT_COUNT];
	double statuses[STATUS_COUNT];
	double fixed;
	double bounceRate;
	double critRate;
	double statusProcRate[STATUS_COUNT];
	double killFrequency;
	double weakSpotRatio;
	double stunRate;

	Dps();

	void clear();
	void computeNoStatus(const MonsterHitData &hit_data,
	                     const DamageData &normal_damage,
	                     const DamageData &weak_damage);
	void computeStatus(const Monster &monster,
	                   const DamageData &normal_damage,
	                   const DamageData &weak_damage,
	                   double defense_multiplier,
	                   double status_defense_multiplier,
	                   double status_hit_multiplier);

	void compute(const Target &target, const Damage &damage);

	void combineNoStatus(const Dps &o, double rate);
	void combine(const Dps &o, double rate);

	double totalDps() const;
	double totalElements() const;
	double totalStatuses() const;

	void print(QTextStream &stream, QString indent = QString());
};

#endif
