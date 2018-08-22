#ifndef DamageData_h_
#define DamageData_h_

#include <QVector>
#include <QString>
#include "enums.h"
#include "FoldedBuffs.h"

struct Weapon;
struct FoldedBuffsData;
struct Pattern;
class QTextStream;

struct SharpnessMultiplierData {
	SharpnessMultiplierData() : rate(0.0), multiplier(0.0) {}
	SharpnessMultiplierData(double r, double m) : rate(r), multiplier(m) {}
	double rate;
	double multiplier;
};

struct DamageData {
	double cut;
	double impact;
	double piercing;
	double bullet;
	double fixed;
	double elements[ELEMENT_COUNT];
	double statuses[STATUS_COUNT];
	double mindsEyeRate;
	double critRate;
	QVector<SharpnessMultiplierData> bounceSharpness;
	FoldedBuffsData *buffData;
	double totalRate;

	DamageData();
	DamageData(const DamageData &o);
	~DamageData();
	DamageData(const Weapon &weapon, const FoldedBuffsData &buffs,
	           const Pattern &pattern,
	           double sharpness_use, double sharpen_period);
	DamageData &operator=(const DamageData &o);
	void setBuffs(const FoldedBuffsData &buffs);
	void clear();
	void combine(const DamageData &o, double rate);
	void print(QTextStream &stream, QString indent = QString()) const;
};

#endif
