#include "ConditionRatios.h"

#include <QTextStream>

ConditionRatios::ConditionRatios()
{
	for (int i = 0; i < CONDITION_COUNT; ++i) {
		ratios[i] = 0.0;
	}
}

void ConditionRatios::print(QTextStream &stream, QString indent) const
{
	for(int i = 0; i < CONDITION_COUNT; ++i) {
		stream << indent << "- " << toString((Condition)i) <<
			": " << ratios[i] << endl;
	}
}

