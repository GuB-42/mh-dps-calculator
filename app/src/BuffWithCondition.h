#ifndef BuffWithCondition_h_
#define BuffWithCondition_h_

#include "enums.h"
#include <QString>

class QTextStream;
struct Weapon;
struct Profile;

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

	enum BuffCombineOp {
		OP_NONE,
		OP_AFFINITY,
		OP_PLUS,
		OP_MULTIPLY,
		OP_MAX
	};

	BuffWithCondition();
	bool isUseful(const Weapon &weapon, const Profile &profile) const;
	bool sameBuffAs(const BuffWithCondition &o) const;
	bool operator<(const BuffWithCondition &o) const;
	BuffCombineOp combineOp() const;
	void print(QTextStream &stream, QString indent = QString()) const;
};

#endif

