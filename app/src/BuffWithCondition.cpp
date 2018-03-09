#include "BuffWithCondition.h"

#include <QTextStream>

BuffWithCondition::BuffWithCondition() :
	buffClass(BUFF_CLASS_NONE), condition(CONDITION_ALWAYS), value(0.0)
{
}

void BuffWithCondition::print(QTextStream &stream, QString indent) const {
	stream << indent << toString(condition) << ": ";
	switch (buffClass) {
	case BUFF_CLASS_NONE:
		stream << "<none>" << endl;
		break;
	case BUFF_CLASS_NORMAL:
		stream << toString(normal.buff) << ": " << value << endl;
		break;
	case BUFF_CLASS_ELEMENT:
		stream << toString(element.buff) << " / " <<
			toString(element.type) << ": " << value << endl;
		break;
	case BUFF_CLASS_STATUS:
		stream << toString(status.buff) << " / " <<
			toString(status.type) << ": " << value << endl;
		break;
	}
}

