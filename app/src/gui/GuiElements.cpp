#include "GuiElements.h"

#include <QLinearGradient>
#include "../BuffWithCondition.h"

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
	case ELEMENT_COUNT: return QColor();
	}
	return QColor();
}

QBrush GuiElements::elementBrush(const double (&v)[ELEMENT_COUNT]) {
	int ecount = 0;
	ElementType elast = ELEMENT_COUNT;
	for (int i = 0; i < ELEMENT_COUNT; ++i) {
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
		for (int i = 0; i < ELEMENT_COUNT; ++i) {
			if (v[i] > 0.0) {
				grad.setColorAt(pos, elementColor((ElementType)i));
				pos += 1.0 / (double)(ecount - 1);
			}
		}
		return QBrush(grad);
	}
}

QString GuiElements::elementName(ElementType e) {
	switch (e) {
	case ELEMENT_FIRE: return tr("Fire");
	case ELEMENT_THUNDER: return tr("Thunder");
	case ELEMENT_ICE: return tr("Ice");
	case ELEMENT_DRAGON: return tr("Dragon");
	case ELEMENT_WATER: return tr("Water");
	case ELEMENT_COUNT: return QString();
	}
	return QString();
}

QString GuiElements::elementToolTip(const double (&v)[ELEMENT_COUNT]) {
	QString ret;
	for (int i = 0; i < ELEMENT_COUNT; ++i) {
		if (v[i] > 0.0) {
			if (!ret.isEmpty()) ret += tr(", ");
			ret += elementName((ElementType)i);
		}
	}
	return ret;
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
	case STATUS_COUNT: return QColor();
	}
	return QColor();
}

QBrush GuiElements::statusBrush(const double (&v)[STATUS_COUNT]) {
	int scount = 0;
	StatusType slast = STATUS_COUNT;
	for (int i = 0; i < STATUS_COUNT; ++i) {
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
		for (int i = 0; i < STATUS_COUNT; ++i) {
			if (v[i] > 0.0 && i != STATUS_STUN &&
			    i != STATUS_EXHAUST && i != STATUS_MOUNT) {
				grad.setColorAt(pos, statusColor((StatusType)i));
				pos += 1.0 / (double)(scount - 1);
			}
		}
		return QBrush(grad);
	}
}

QString GuiElements::statusName(StatusType t) {
	switch (t) {
	case STATUS_POISON: return tr("Poison");
	case STATUS_PARALYSIS: return tr("Paralysis");
	case STATUS_SLEEP: return tr("Sleep");
	case STATUS_STUN: return tr("Stun");
	case STATUS_BLAST: return tr("Blast");
	case STATUS_EXHAUST: return tr("Exhaust");
	case STATUS_MOUNT: return tr("Mount");
	case STATUS_COUNT: return QString();
	}
	return QString();
}

QString GuiElements::statusToolTip(const double (&v)[STATUS_COUNT]) {
	QString ret;
	for (int i = 0; i < STATUS_COUNT; ++i) {
		if (v[i] > 0.0 && i != STATUS_STUN &&
		    i != STATUS_EXHAUST && i != STATUS_MOUNT) {
			if (!ret.isEmpty()) ret += tr(", ");
			ret += statusName((StatusType)i);
		}
	}
	return ret;
}

QString GuiElements::conditionName(Condition c) {
	switch (c) {
	case CONDITION_ALWAYS: return tr("Always");
	case CONDITION_ENRAGED: return tr("Enraged");
	case CONDITION_WEAK_SPOT: return tr("Weak spot");
	case CONDITION_RAW_WEAPON: return tr("Raw weapon");
	case CONDITION_DRAW_ATTACK: return tr("Draw attack");
	case CONDITION_AIRBORNE: return tr("Airborne");
	case CONDITION_RED_LIFE: return tr("Red life (resentment)");
	case CONDITION_FULL_LIFE: return tr("Full life");
	case CONDITION_LOW_LIFE: return tr("Low life (heroics)");
	case CONDITION_DEATH_1: return tr("Fainted once");
	case CONDITION_DEATH_2: return tr("Fainted twice");
	case CONDITION_FULL_STAMINA: return tr("Full stamina");
	case CONDITION_SLIDING: return tr("Sliding");
	case CONDITION_RECEIVED_DAMAGE: return tr("Received damage (latent power)");
	case CONDITION_DEMON_POWDER_USE: return tr("Using demon powder");
	case CONDITION_MIGHT_SEED_USE: return tr("Using might seed");
	case CONDITION_MIGHT_PILL_USE: return tr("Using might pill");
	case CONDITION_SONG_BASE: return tr("Active song (base)");
	case CONDITION_SONG_ENCORE: return tr("Active song (encore)");
	case CONDITION_COUNT: return QString();
	}
	return QString();
}

QString GuiElements::normalBuffName(NormalBuff buff) {
	switch (buff) {
	case BUFF_ATTACK_PLUS: return tr("Attack plus");
	case BUFF_ATTACK_MULTIPLIER: return tr("Attack multiplier");
	case BUFF_AFFINITY_PLUS: return tr("Affinity plus");
	case BUFF_ALL_ELEMENTS_PLUS: return tr("All elements plus");
	case BUFF_ALL_ELEMENTS_MULTIPLIER: return tr("All elements multiplier");
	case BUFF_ALL_STATUSES_PLUS: return tr("All statuses plus");
	case BUFF_ALL_STATUSES_MULTIPLIER: return tr("All statuses multiplier");
	case BUFF_AWAKENING: return tr("Awakening");
	case BUFF_SHARPNESS_PLUS: return tr("Sharpness plus");
	case BUFF_MAX_SHARPNESS_TIME: return tr("Time at max sharpness");
	case BUFF_SHARPNESS_USE_MULTIPLIER: return tr("Sharpness use multiplier");
	case BUFF_SHARPNESS_USE_CRITICAL_MULTIPLIER: return tr("Sharpness use for critical hits");
	case BUFF_PUNISHING_DRAW: return tr("Punishing draw");
	case BUFF_STUN_MULTIPLIER: return tr("Stun multiplier");
	case BUFF_EXHAUST_MULTIPLIER: return tr("Exhaust multiplier");
	case BUFF_MOUNT_MULTIPLIER: return tr("Mount multiplier");
	case BUFF_ARTILLERY_MULTIPLIER: return tr("Artillery multiplier");
	case BUFF_RAW_CRITICAL_HIT_MULTIPLIER: return tr("Critical hit multiplier (raw)");
	case BUFF_ELEMENT_CRITICAL_HIT_MULTIPLIER: return tr("Critical hit multiplier (element)");
	case BUFF_STATUS_CRITICAL_HIT_MULTIPLIER: return tr("Critical hit multiplier (status)");
	case BUFF_MINDS_EYE: return tr("Mind's eye factor");
	case BUFF_ATTACK_PLUS_BEFORE: return tr("Base attack plus");
	case BUFF_CAPACITY_UP: return tr("Capacity up");
	case NORMAL_BUFF_COUNT: return QString();
	}
	return QString();
}

QString GuiElements::elementBuffName(ElementBuff buff, ElementType type) {
	switch (buff) {
	case BUFF_ELEMENT_PLUS:
		switch (type) {
		case ELEMENT_FIRE: return tr("Fire plus");
		case ELEMENT_THUNDER: return tr("Thunder plus");
		case ELEMENT_ICE: return tr("Ice plus");
		case ELEMENT_DRAGON: return tr("Dragon plus");
		case ELEMENT_WATER: return tr("Water plus");
		case ELEMENT_COUNT: return QString();
		}
		return QString();
	case BUFF_ELEMENT_MULTIPLIER:
		switch (type) {
		case ELEMENT_FIRE: return tr("Fire multiplier");
		case ELEMENT_THUNDER: return tr("Thunder multiplier");
		case ELEMENT_ICE: return tr("Ice multiplier");
		case ELEMENT_DRAGON: return tr("Dragon multiplier");
		case ELEMENT_WATER: return tr("Water multiplier");
		case ELEMENT_COUNT: return QString();
		}
		return QString();
	case ELEMENT_BUFF_COUNT: return QString();
	}
	return QString();
}

QString GuiElements::statusBuffName(StatusBuff buff, StatusType type) {
	switch (buff) {
	case BUFF_STATUS_PLUS:
		switch (type) {
		case STATUS_POISON: return tr("Poison plus");
		case STATUS_PARALYSIS: return tr("Paralysis plus");
		case STATUS_SLEEP: return tr("Sleep plus");
		case STATUS_STUN: return tr("Stun plus");
		case STATUS_BLAST: return tr("Blast plus");
		case STATUS_EXHAUST: return tr("Exhaust plus");
		case STATUS_MOUNT: return tr("Mount plus");
		case STATUS_COUNT: return QString();
		}
		return QString();
	case BUFF_STATUS_MULTIPLIER:
		switch (type) {
		case STATUS_POISON: return tr("Poison multiplier");
		case STATUS_PARALYSIS: return tr("Paralysis multiplier");
		case STATUS_SLEEP: return tr("Sleep multiplier");
		case STATUS_STUN: return tr("Stun multiplier");
		case STATUS_BLAST: return tr("Blast multiplier");
		case STATUS_EXHAUST: return tr("Exhaust multiplier");
		case STATUS_MOUNT: return tr("Mount multiplier");
		case STATUS_COUNT: return QString();
		}
		return QString();
	case STATUS_BUFF_COUNT: return QString();
	}
	return QString();
}
