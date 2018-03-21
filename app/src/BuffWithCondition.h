#ifndef BuffWithCondition_h_
#define BuffWithCondition_h_

#include "enums.h"
#include <QString>

class QTextStream;
class Weapon;

struct BuffWithCondition {
	enum BuffClass {
		BUFF_CLASS_NONE,
		BUFF_CLASS_NORMAL,
		BUFF_CLASS_ELEMENT,
		BUFF_CLASS_STATUS
	};

	BuffClass buffClass;
	Condition condition;
	double value;
	union {
		struct {
			NormalBuff buff;
		} normal;
		struct {
			ElementBuff buff;
			ElementType type;
		} element;
		struct {
			StatusBuff buff;
			StatusType type;
		} status;
	};

	BuffWithCondition();
	bool isUseful(const Weapon &weapon) const;
	void print(QTextStream &stream, QString indent = QString()) const;
};

#endif

