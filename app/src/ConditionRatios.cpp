#include "ConditionRatios.h"

#include <QTextStream>
#include "QtCompatibility.h"

ConditionRatios::ConditionRatios()
{
	std::fill_n(ratios, (size_t)CONDITION_COUNT, 0.0);
}

void ConditionRatios::print(QTextStream &stream, QString indent) const
{
	for(int i = 0; i < CONDITION_COUNT; ++i) {
		stream << indent << "- " << toString((Condition)i) <<
			": " << ratios[i] << ENDL;
	}
}

