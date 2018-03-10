#ifndef ConditionRatios_h_
#define ConditionRatios_h_

#include "enums.h"
#include <QString>

class QTextStream;

struct ConditionRatios {
	double ratios[CONDITION_COUNT];

	ConditionRatios();
	inline double &operator[](Condition c) { return ratios[c]; }
	inline const double &operator[](Condition c) const { return ratios[c]; }
	void print(QTextStream &stream, QString indent = QString()) const;
};

#endif

