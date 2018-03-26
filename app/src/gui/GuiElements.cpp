#include "GuiElements.h"

#include <QLinearGradient>

QColor GuiElements::elementColor(ElementType e) {
	const int full = 255;
	const int mid2 = 256 - 32;
	const int mid1 = 256 - 48;
	const int low = 256 - 64;
	switch (e) {
	case ELEMENT_FIRE: return QColor(full, mid1, low);
	case ELEMENT_THUNDER: return QColor(full, full, low);
	case ELEMENT_ICE: return QColor(mid2, full, full);
	case ELEMENT_DRAGON: return QColor(mid2, full, mid2);
	case ELEMENT_WATER: return QColor(low, mid2, full);
	default: return QColor();
	}
}

QBrush GuiElements::elementBrush(const double (&v)[ElementType::ELEMENT_COUNT]) {
	int ecount = 0;
	ElementType elast = ELEMENT_COUNT;
	for (int i = 0; i < ElementType::ELEMENT_COUNT; ++i) {
		if (v[i] > 0.0) {
			++ecount;
			elast = (ElementType)i;
		}
	}
	if (ecount == 0) {
		return QBrush();
	} else if (ecount == 1) {
		return QBrush(elementColor(elast));
	} else {
		QLinearGradient grad(0, 0, 10, 10);
		grad.setSpread(QGradient::ReflectSpread);
		double pos = 0.0;
		for (int i = 0; i < ElementType::ELEMENT_COUNT; ++i) {
			if (v[i] > 0.0) {
				grad.setColorAt(pos, elementColor((ElementType)i));
				pos += 1.0 / (double)(ecount - 1);
			}
		}
		return QBrush(grad);
	}
}

QColor GuiElements::statusColor(StatusType t) {
	const int full = 255;
	const int mid3 = 256 - 16;
	const int mid2 = 256 - 32;
	const int mid1 = 256 - 48;
	const int low = 256 - 64;
	switch (t) {
	case STATUS_POISON: return QColor(mid3, mid1, full);
	case STATUS_PARALYSIS: return QColor(mid3, mid3, low);
	case STATUS_SLEEP: return QColor(mid2, mid2, full);
	case STATUS_STUN: return QColor(full, mid2, low);
	case STATUS_BLAST: return QColor(full, mid1, mid1);
	case STATUS_EXHAUST: return QColor(low, mid1, full);
	case STATUS_MOUNT: return QColor(mid1, mid3, mid1);
	default: return QColor();
	}
}

QBrush GuiElements::statusBrush(const double (&v)[StatusType::STATUS_COUNT]) {
	int scount = 0;
	StatusType slast = STATUS_COUNT;
	for (int i = 0; i < StatusType::STATUS_COUNT; ++i) {
		if (v[i] > 0.0 && i != STATUS_STUN &&
		    i != STATUS_EXHAUST && i != STATUS_MOUNT) {
			++scount;
			slast = (StatusType)i;
		}
	}
	if (scount == 0) {
		return QBrush();
	} else if (scount == 1) {
		return QBrush(statusColor(slast));
	} else {
		QLinearGradient grad(0, 0, 10, 10);
		grad.setSpread(QGradient::ReflectSpread);
		double pos = 0.0;
		for (int i = 0; i < StatusType::STATUS_COUNT; ++i) {
			if (v[i] > 0.0 && i != STATUS_STUN &&
			    i != STATUS_EXHAUST && i != STATUS_MOUNT) {
				grad.setColorAt(pos, statusColor((StatusType)i));
				pos += 1.0 / (double)(scount - 1);
			}
		}
		return QBrush(grad);
	}
}
