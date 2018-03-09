#ifndef ConditionRatios_h_
#define ConditionRatios_h_

#include "enums.h"
#include <QString>

class QTextStream;

struct ConditionRatios {
	double ratios[CONDITION_COUNT];

	ConditionRatios();
	double &operator[](Condition c);
	const double &operator[](Condition c) const;
	void print(QTextStream &stream, QString indent = QString()) const;
};

#endif

