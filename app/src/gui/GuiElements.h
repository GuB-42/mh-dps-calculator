#ifndef GuiElements_h_
#define GuiElements_h_

#include <QCoreApplication>
#include <QBrush>
#include <QColor>
#include "../enums.h"

class GuiElements {
	Q_DECLARE_TR_FUNCTIONS(GuiElements);

public:
	static QColor elementColor(ElementType e);
	static QBrush elementBrush(const double (&v)[ELEMENT_COUNT]);
	static QString elementName(ElementType e);
	static QString elementToolTip(const double (&v)[ELEMENT_COUNT]);
	static QColor statusColor(StatusType t);
	static QBrush statusBrush(const double (&v)[STATUS_COUNT]);
	static QString statusName(StatusType t);
	static QString statusToolTip(const double (&v)[STATUS_COUNT]);
	static QString conditionName(Condition c);
	static QString normalBuffName(NormalBuff buff);
	static QString elementBuffName(ElementBuff buff, ElementType type);
	static QString statusBuffName(StatusBuff buff, StatusType type);
};

#endif
