#ifndef Damage_h_
#define Damage_h_

#include "enums.h"
#include <QVector>
#include <QString>

class QTextStream;
struct DamageData;
struct BuffWithCondition;
struct Weapon;
struct Pattern;
struct FoldedBuffs;

struct Damage {
	DamageData *data[MODE_COUNT];
	double sharpenPeriod;
	double sharpnessUse[MODE_COUNT];
	bool isAlias[MODE_COUNT];

	Damage();
	~Damage();
	void addSharpnessUse(const FoldedBuffs &folded_buffs,
	                     const Weapon &weapon, const Pattern &pattern,
	                     double base_rate);
	void addPattern(const FoldedBuffs &folded_buffs,
	                const Weapon &weapon, const Pattern &pattern,
	                double base_rate);
	void print(QTextStream &stream, QString indent = QString()) const;
};

#endif
