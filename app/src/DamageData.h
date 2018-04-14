#ifndef DamageData_h_
#define DamageData_h_

#include <QVector>
#include <QString>
#include "enums.h"

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
	double fixed;
	double elements[ELEMENT_COUNT];
	double statuses[STATUS_COUNT];
	double mindsEyeRate;
	QVector<SharpnessMultiplierData> bounceSharpness;

	DamageData();
	DamageData(const Weapon &weapon, const FoldedBuffsData &buffs,
	           const Pattern &pattern,
	           double sharpness_use, double sharpen_period);
	void combine(const DamageData &o, double rate);
	void print(QTextStream &stream, QString indent = QString()) const;
};

#endif
