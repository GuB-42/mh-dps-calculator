#ifndef GuiElements_h_
#define GuiElements_h_

#include <QBrush>
#include <QColor>
#include "../enums.h"

class GuiElements {
public:
	static QColor elementColor(ElementType e);
	static QBrush elementBrush(const double (&v)[ELEMENT_COUNT]);
	static QColor statusColor(StatusType t);
	static QBrush statusBrush(const double (&v)[STATUS_COUNT]);
};

#endif
