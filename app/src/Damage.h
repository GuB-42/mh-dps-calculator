#ifndef Damage_h_
#define Damage_h_

#include "enums.h"
#include <QVector>
#include <QString>

class DamageData;
class BuffWithCondition;
class Weapon;
class Pattern;
class QTextStream;

struct Damage {
	DamageData *data[MODE_COUNT];
	bool isAlias[MODE_COUNT];

	Damage();
	~Damage();
	void addPattern(const QVector<const BuffWithCondition *> &buff_conds,
	                const Weapon &weapon, const Pattern &pattern);
	void print(QTextStream &stream, QString indent = QString()) const;
};

#endif
