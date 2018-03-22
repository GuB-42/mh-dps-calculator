#include <QBrush>
#include <QColor>
#include "../enums.h"

class GuiElements {
public:
	static QColor elementColor(ElementType e);
	static QBrush elementBrush(const double (&v)[ElementType::ELEMENT_COUNT]);
	static QColor statusColor(StatusType t);
	static QBrush statusBrush(const double (&v)[StatusType::STATUS_COUNT]);
};
