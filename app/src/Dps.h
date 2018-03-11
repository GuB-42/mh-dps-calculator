#ifndef Dps_h_
#define Dps_h_

#include <QString>
#include "enums.h"

class QTextStream;
class Monster;
class MonsterHitData;
class DamageData;

struct Dps {
	double total;
	double raw;
	double total_elements;
	double elements[ELEMENT_COUNT];
	double total_statuses;
	double statuses[STATUS_COUNT];
	double fixed;
	double bounceRate;
	double statusProcRate[STATUS_COUNT];
	double killFrequency;

	Dps();
	Dps(const Monster &monster,
	    const MonsterHitData &hit_data,
	    const DamageData &normal_damage,
	    const DamageData &weak_damage,
	    double defense_multiplier);
	void combine(const Dps &o, double rate);
	void print(QTextStream &stream, QString indent = QString());
};

#endif
