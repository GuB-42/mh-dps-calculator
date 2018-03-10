#ifndef enums_h_
#define enums_h_

enum StatusType {
	STATUS_POISON,
	STATUS_PARALYSIS,
	STATUS_SLEEP,
	STATUS_STUN,
	STATUS_BLAST,
	STATUS_EXHAUST,
	STATUS_MOUNT,
	STATUS_COUNT
};
const char *toString(StatusType t);

enum ElementType {
	ELEMENT_FIRE,
	ELEMENT_THUNDER,
	ELEMENT_ICE,
	ELEMENT_DRAGON,
	ELEMENT_WATER,
	ELEMENT_COUNT
};
const char *toString(ElementType t);

enum NormalBuff {
	BUFF_ATTACK_PLUS,
	BUFF_ATTACK_MULTIPLIER,
	BUFF_AFFINITY_PLUS,
	BUFF_ALL_ELEMENTS_PLUS,
	BUFF_ALL_ELEMENTS_MULTIPLIER,
	BUFF_ALL_STATUSES_PLUS,
	BUFF_ALL_STATUSES_MULTIPLIER,
	BUFF_AWAKENING,
	BUFF_SHARPNESS_PLUS,
	BUFF_MAX_SHARPNESS_TIME,
	BUFF_SHARPNESS_USE_MULTIPLIER,
	BUFF_SHARPNESS_USE_CRITICAL_MULTIPLIER,
	BUFF_DRAW_ATTACK_STUN,
	BUFF_DRAW_ATTACK_EXHAUST,
	BUFF_STUN_MULTIPLIER,
	BUFF_EXHAUST_MULTIPLIER,
	BUFF_ARTILLERY_MULTIPLIER,
	BUFF_RAW_CRITICAL_HIT_MULTIPLIER,
	BUFF_ELEMENT_CRITICAL_HIT_MULTIPLIER,
	BUFF_STATUS_CRITICAL_HIT_MULTIPLIER,
	BUFF_MINDS_EYE,
	NORMAL_BUFF_COUNT
};
const char *toString(NormalBuff b);

enum ElementBuff {
	BUFF_ELEMENT_PLUS,
	BUFF_ELEMENT_MULTIPLIER,
	ELEMENT_BUFF_COUNT
};
const char *toString(ElementBuff b);

enum StatusBuff {
	BUFF_STATUS_PLUS,
	BUFF_STATUS_MULTIPLIER,
	STATUS_BUFF_COUNT
};
const char *toString(StatusBuff b);

enum Condition {
	CONDITION_ALWAYS,
	CONDITION_ENRAGED,
	CONDITION_WEAK_SPOT,
	CONDITION_RAW_WEAPON,
	CONDITION_DRAW_ATTACK,
	CONDITION_AIRBORNE,
	CONDITION_RED_LIFE,
	CONDITION_FULL_LIFE,
	CONDITION_DEATH_1,
	CONDITION_DEATH_2,
	CONDITION_FULL_STAMINA,
	CONDITION_SLIDING,
	CONDITION_LOW_LIFE,
	CONDITION_RECEIVED_DAMAGE,
	CONDITION_MIGHT_SEED_USE,
	CONDITION_DEMON_POWDER_USE,
	CONDITION_MIGHT_PILL_USE,
	CONDITION_COUNT
};
const char *toString(Condition c);

enum PhialType {
	PHIAL_NONE,
	PHIAL_ELEMENT,
	PHIAL_IMPACT,
	PHIAL_POWER,
	PHIAL_COUNT
};
const char *toString(PhialType t);

enum SharpnessLevel {
	SHARPNESS_RED,
	SHARPNESS_ORANGE,
	SHARPNESS_YELLOW,
	SHARPNESS_GREEN,
	SHARPNESS_BLUE,
	SHARPNESS_WHITE,
	SHARPNESS_PURPLE,
	SHARPNESS_COUNT
};
const char *toString(SharpnessLevel s);

#endif
