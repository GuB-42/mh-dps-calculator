#include "enums.h"

const char *toString(StatusType t) {
	switch (t) {
	case STATUS_POISON: return "poison";
	case STATUS_PARALYSIS: return "paralysis";
	case STATUS_SLEEP: return "sleep";
	case STATUS_STUN: return "stun";
	case STATUS_BLAST: return "blast";
	case STATUS_EXHAUST: return "exhaust";
	case STATUS_MOUNT: return "mount";
	default: return "???";
	}
}

const char *toString(ElementType t) {
	switch (t) {
	case ELEMENT_FIRE: return "fire";
	case ELEMENT_THUNDER: return "thunder";
	case ELEMENT_ICE: return "ice";
	case ELEMENT_DRAGON: return "dragon";
	case ELEMENT_WATER: return "water";
	default: return "???";
	}
}

const char *toString(NormalBuff b) {
	switch (b) {
	case BUFF_ATTACK_PLUS: return "attack_plus";
	case BUFF_ATTACK_MULTIPLIER: return "attack_multiplier";
	case BUFF_AFFINITY_PLUS: return "affinity_plus";
	case BUFF_ALL_ELEMENTS_PLUS: return "all_elements_plus";
	case BUFF_ALL_ELEMENTS_MULTIPLIER: return "all_elements_multiplier";
	case BUFF_ALL_STATUSES_PLUS: return "all_statuses_plus";
	case BUFF_ALL_STATUSES_MULTIPLIER: return "all_statuses_multiplier";
	case BUFF_AWAKENING: return "awakening";
	case BUFF_SHARPNESS_PLUS: return "sharpness_plus";
	case BUFF_MAX_SHARPNESS_TIME: return "max_sharpness_time";
	case BUFF_SHARPNESS_USE_MULTIPLIER: return "sharpness_use_multiplier";
	case BUFF_SHARPNESS_USE_CRITICAL_MULTIPLIER: return "sharpness_use_critical_multiplier";
	case BUFF_DRAW_ATTACK_STUN: return "draw_attack_stun";
	case BUFF_DRAW_ATTACK_EXHAUST: return "draw_attack_exhaust";
	case BUFF_STUN_MULTIPLIER: return "stun_multiplier";
	case BUFF_EXHAUST_MULTIPLIER: return "exhaust_multiplier";
	case BUFF_ARTILLERY_MULTIPLIER: return "artillery_multiplier";
	case BUFF_RAW_CRITICAL_HIT_MULTIPLIER: return "raw_critical_hit_multiplier";
	case BUFF_ELEMENT_CRITICAL_HIT_MULTIPLIER: return "element_critical_hit_multiplier";
	case BUFF_STATUS_CRITICAL_HIT_MULTIPLIER: return "status_critical_hit_multiplier";
	case BUFF_MINDS_EYE: return "minds_eye";
	case BUFF_ATTACK_PLUS_BEFORE: return "attack_plus_before";
	default: return "???";
	}
}

const char *toString(ElementBuff b) {
	switch (b) {
	case BUFF_ELEMENT_PLUS: return "element_plus";
	case BUFF_ELEMENT_MULTIPLIER: return "element_multiplier";
	default: return "???";
	}
}

const char *toString(StatusBuff b) {
	switch (b) {
	case BUFF_STATUS_PLUS: return "status_plus";
	case BUFF_STATUS_MULTIPLIER: return "status_multiplier";
	default: return "???";
	}
}

const char *toString(Condition c) {
	switch (c) {
	case CONDITION_ALWAYS: return "always";
	case CONDITION_ENRAGED: return "enraged";
	case CONDITION_WEAK_SPOT: return "weak_spot";
	case CONDITION_RAW_WEAPON: return "raw_weapon";
	case CONDITION_DRAW_ATTACK: return "draw_attack";
	case CONDITION_AIRBORNE: return "airborne";
	case CONDITION_RED_LIFE: return "red_life";
	case CONDITION_FULL_LIFE: return "full_life";
	case CONDITION_DEATH_1: return "death_1";
	case CONDITION_DEATH_2: return "death_2";
	case CONDITION_FULL_STAMINA: return "full_stamina";
	case CONDITION_SLIDING: return "sliding";
	case CONDITION_LOW_LIFE: return "low_life";
	case CONDITION_RECEIVED_DAMAGE: return "received_damage";
	case CONDITION_MIGHT_SEED_USE: return "might_seed_use";
	case CONDITION_DEMON_POWDER_USE: return "demon_powder_use";
	case CONDITION_MIGHT_PILL_USE: return "might_pill_use";
	default: return "???";
	}
}

const char *toString(PhialType t) {
	switch (t) {
	case PHIAL_NONE: return "no_phial";
	case PHIAL_ELEMENT: return "element";
	case PHIAL_IMPACT: return "impact";
	case PHIAL_POWER: return "power";
	default: return "???";
	}
}

const char *toString(SharpnessLevel t) {
	switch (t) {
	case SHARPNESS_RED: return "red";
	case SHARPNESS_ORANGE: return "orange";
	case SHARPNESS_YELLOW: return "yellow";
	case SHARPNESS_GREEN: return "green";
	case SHARPNESS_BLUE: return "blue";
	case SHARPNESS_WHITE: return "white";
	case SHARPNESS_PURPLE: return "purple";
	default: return "???";
	}
}

const char *toString(MonsterMode m) {
	switch (m) {
	case MODE_NORMAL_NORMAL: return "normal_normal";
	case MODE_NORMAL_WEAK_SPOT: return "normal_weak_spot";
	case MODE_ENRAGED_NORMAL: return "enraged_normal";
	case MODE_ENRAGED_WEAK_SPOT: return "enraged_weak_spot";
	default: return "???";
	}
}
