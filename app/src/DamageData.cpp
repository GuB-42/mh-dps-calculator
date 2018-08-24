#include "DamageData.h"

#include <QTextStream>
#include "Weapon.h"
#include "WeaponType.h"
#include "Profile.h"
#include "Constants.h"

DamageData::DamageData() :
	cut(0.0), impact(0.0), piercing(0.0), bullet(0.0), fixed(0.0),
	mindsEyeRate(0.0), critRate(0.0),
	buffData(NULL), totalRate(0.0)
{
	std::fill_n(elements, (size_t)ELEMENT_COUNT, 0.0);
	std::fill_n(statuses, (size_t)STATUS_COUNT, 0.0);
}

DamageData::DamageData(const DamageData &o) :
	cut(o.cut), impact(o.impact), piercing(o.piercing),
	bullet(o.bullet), fixed(o.fixed),
	mindsEyeRate(o.mindsEyeRate), critRate(o.critRate),
	bounceSharpness(o.bounceSharpness),
	buffData(NULL), totalRate(o.totalRate)
{
	std::copy(o.elements, o.elements + ELEMENT_COUNT, elements);
	std::copy(o.statuses, o.statuses + STATUS_COUNT, statuses);
	if (o.buffData) buffData = new FoldedBuffsData(*o.buffData);
}

DamageData::~DamageData() {
	delete buffData;
}

double compute_sharp_bonus(const double levels[SHARPNESS_COUNT],
                           double wasted, double use, double max_ratio,
                           const double values[SHARPNESS_COUNT],
                           DamageData::BounceSharpnessArray *pmult = NULL) {
	int i = SHARPNESS_COUNT - 1;
	double cur_level = levels[i];

	while (i >= 0 && cur_level == 0.0) cur_level = levels[--i];

	while (wasted >= cur_level && i > 0) {
		wasted -= cur_level;
		cur_level = levels[--i];
	}
	cur_level -= wasted;

	if (pmult) pmult->resize(SHARPNESS_COUNT);
	size_t md_idx = 0;
	double ret = 0.0;
	if (i < 0) {
		if (pmult) {
			(*pmult)[md_idx].rate = 1.0;
			(*pmult)[md_idx++].multiplier = values[0];
		}
		ret = values[0];
	} else if (use <= 0.0 || max_ratio >= 1.0) {
		if (pmult) {
			(*pmult)[md_idx].rate = 1.0;
			(*pmult)[md_idx++].multiplier = values[i];
		}
		ret = values[i];
	} else {
		double remaining_max = max_ratio;
		double remaining_use = use * (1.0 - max_ratio);
		while (remaining_use >= cur_level && i > 0) {
			double r = remaining_max + cur_level / use;
			ret += r * values[i];
			if (r > 0.0 && pmult) {
				(*pmult)[md_idx].rate = r;
				(*pmult)[md_idx++].multiplier = values[i];
			}
			remaining_use -= cur_level;
			cur_level = levels[--i];
			remaining_max = 0.0;
		}
		double r = remaining_max + remaining_use / use;
		ret += r * values[i];
		if (r > 0.0 && pmult) {
			(*pmult)[md_idx].rate = r;
			(*pmult)[md_idx++].multiplier = values[i];
		}
	}
	if (pmult) pmult->resize(md_idx);
	return ret;
};

static double compute_buffed_element(double base,
                                     double plus, double multiplier,
                                     const QVector<BuffCap> &caps)
{
	double ret = (base + plus) * multiplier;
	double cap = 0.0;
	foreach(const BuffCap &bc, caps) {
		if (bc.base <= base) {
			cap = bc.cap;
		} else {
			break;
		}
	}
	return ret > base + cap ? base + cap : ret;
}

// TODO: check if exhaust switch axe phials also deals KO
// and also how it stacks with awakening
DamageData::DamageData(const Weapon &weapon, const FoldedBuffsData &buffs,
                       const Pattern &pattern,
                       double sharpness_use, double sharpen_period) :
	buffData(NULL), totalRate(0.0)
{
	double pre_attack =
		weapon.attack + buffs.normalBuffs[BUFF_ATTACK_PLUS_BEFORE];
	double attack = (pre_attack * buffs.normalBuffs[BUFF_ATTACK_MULTIPLIER]) +
		buffs.normalBuffs[BUFF_ATTACK_PLUS];

	double xaffinity =
		(weapon.affinity + buffs.normalBuffs[BUFF_AFFINITY_PLUS]) / 100.0;

	double raw_affinity_multiplier = 1.0;
	double element_affinity_multiplier = 1.0;
	double status_affinity_multiplier = 1.0;
	if (xaffinity > 0.0) {
		raw_affinity_multiplier = (1.0 - xaffinity) +
			xaffinity * buffs.normalBuffs[BUFF_RAW_CRITICAL_HIT_MULTIPLIER];
		element_affinity_multiplier = (1.0 - xaffinity) +
			xaffinity * buffs.normalBuffs[BUFF_ELEMENT_CRITICAL_HIT_MULTIPLIER];
		status_affinity_multiplier = (1.0 - xaffinity) +
			xaffinity * buffs.normalBuffs[BUFF_STATUS_CRITICAL_HIT_MULTIPLIER];
		if (weapon.type) {
			element_affinity_multiplier = 1.0 +
				(element_affinity_multiplier - 1.0) * weapon.type->elementCritAdjustment;
			status_affinity_multiplier = 1.0 +
				(status_affinity_multiplier - 1.0) * weapon.type->statusCritAdjustment;
		}
	} else if (xaffinity < 0.0) {
		raw_affinity_multiplier = (1.0 + xaffinity) -
			xaffinity * Constants::instance()->feebleHitMultiplier;
	}

	double wasted_sharpness =
		weapon.sharpnessPlus - buffs.normalBuffs[BUFF_SHARPNESS_PLUS];
	if (wasted_sharpness < 0.0) wasted_sharpness = 0.0;
	double max_sharpness_ratio = sharpen_period <= 0.0 ? 1.0 :
		buffs.normalBuffs[BUFF_MAX_SHARPNESS_TIME] / sharpen_period;
	double raw_sharpness_multiplier =
		compute_sharp_bonus(weapon.sharpness, wasted_sharpness,
	                        sharpness_use, max_sharpness_ratio,
	                        Constants::instance()->rawSharpnessMultipliers,
	                        &bounceSharpness);
	for (size_t i = 0; i < bounceSharpness.size(); ++i) {
		bounceSharpness[i].multiplier *= pattern.sharpnessMultiplier;
	}
	double element_sharpness_multiplier =
		compute_sharp_bonus(weapon.sharpness, wasted_sharpness,
	                        sharpness_use, max_sharpness_ratio,
	                        Constants::instance()->elementSharpnessMultipliers);
	// WARN: do maybe redesign, TODO, to check
	raw_sharpness_multiplier *= pattern.sharpnessMultiplier;
	double minds_eye_buff = buffs.normalBuffs[BUFF_MINDS_EYE];
	if (minds_eye_buff < 0.0) minds_eye_buff = 0.0;
	if (minds_eye_buff > 1.0) minds_eye_buff = 1.0;
	mindsEyeRate = minds_eye_buff +
		(1.0 - minds_eye_buff) * pattern.mindsEyeRatio;


	double raw_phial_multiplier = 1.0;
	if (weapon.phial == PHIAL_POWER) {
		raw_phial_multiplier = 1.0 +
			(Constants::instance()->phialPowerBoost - 1.0) * pattern.phialRatio;
	}
	double element_phial_multiplier = 1.0;
	if (weapon.phial == PHIAL_ELEMENT) {
		element_phial_multiplier = 1.0 +
			(Constants::instance()->phialElementBoost - 1.0) * pattern.phialRatio;
	}
	double status_phial_multiplier = element_phial_multiplier;

	double final_raw_attack = attack * raw_affinity_multiplier *
		raw_sharpness_multiplier * raw_phial_multiplier;

	cut = final_raw_attack * pattern.cut;
	impact = final_raw_attack * pattern.impact;
	piercing = final_raw_attack * pattern.piercing;
	bullet = attack * raw_affinity_multiplier * pattern.bullet;
	critRate = xaffinity * (cut + impact + piercing + bullet);

	double multi_element_divider = 0.0;
	for (int elt = 0; elt < ELEMENT_COUNT; ++elt) {
		if (weapon.elements[elt] > 0.0) multi_element_divider += 1.0;
	}
	for (int sta = 0; sta < STATUS_COUNT; ++sta) {
		if (weapon.statuses[sta] > 0.0) multi_element_divider += 1.0;
	}

	double element_multiplier = element_affinity_multiplier *
		element_sharpness_multiplier * element_phial_multiplier;
	for (int elt = 0; elt < ELEMENT_COUNT; ++elt) {
		double element_attack = 0.0;
		double element_phial_attack = 0.0;
		double eplus = buffs.elementBuffs[BUFF_ELEMENT_PLUS][elt] +
			buffs.normalBuffs[BUFF_ALL_ELEMENTS_PLUS];
		double emult = buffs.elementBuffs[BUFF_ELEMENT_MULTIPLIER][elt] *
			buffs.normalBuffs[BUFF_ALL_ELEMENTS_MULTIPLIER];
		if (weapon.elements[elt] > 0.0) {
			double buffed_element =
				compute_buffed_element(weapon.elements[elt], eplus, emult,
				                       Constants::instance()->elementBuffCaps);
			double awake = 1.0;
			if (weapon.awakened) awake = buffs.normalBuffs[BUFF_AWAKENING];
			element_attack = awake *
				pattern.element * buffed_element / multi_element_divider;
			if (weapon.phialElements[elt] > 0.0) {
				element_attack *= (1.0 - pattern.phialRatio);
			}
			if (weapon.phial == PHIAL_ELEMENT) {
				element_phial_attack =
					buffed_element * pattern.phialElementAttack;
			}
		}
		if (weapon.phialElements[elt] > 0.0) {
			element_attack += pattern.phialRatio * pattern.element *
				compute_buffed_element(weapon.phialElements[elt], eplus, emult,
				                       Constants::instance()->elementBuffCaps);
		}
		elements[elt] = element_attack * element_multiplier +
			element_phial_attack;
	}

	double status_multiplier = status_affinity_multiplier *
		Constants::instance()->statusAttackRate * status_phial_multiplier;
	for (int sta = 0; sta < STATUS_COUNT; ++sta) {
		double status_attack = 0.0;
		double splus = buffs.statusBuffs[BUFF_STATUS_PLUS][sta] +
			buffs.normalBuffs[BUFF_ALL_STATUSES_PLUS];
		double smult = buffs.statusBuffs[BUFF_STATUS_MULTIPLIER][sta] *
			buffs.normalBuffs[BUFF_ALL_STATUSES_MULTIPLIER];
		if (weapon.statuses[sta] > 0.0) {
			double buffed_status =
				compute_buffed_element(weapon.statuses[sta], splus, smult,
				                       Constants::instance()->statusBuffCaps);
			double awake = 1.0;
			if (weapon.awakened) awake = buffs.normalBuffs[BUFF_AWAKENING];
			status_attack = awake *
				pattern.status * buffed_status / multi_element_divider;
			if (weapon.phialStatuses[sta] > 0.0) {
				status_attack *= (1.0 - pattern.phialRatio);
			}
		}
		if (weapon.phialStatuses[sta] > 0.0) {
			status_attack += pattern.phialRatio * pattern.status *
				compute_buffed_element(weapon.phialStatuses[sta], splus, smult,
				                       Constants::instance()->statusBuffCaps);
		}
		statuses[sta] = status_attack * status_multiplier;
	}

	statuses[STATUS_STUN] += pattern.stun +
		pattern.punishingDrawStun * buffs.normalBuffs[BUFF_PUNISHING_DRAW];
	statuses[STATUS_EXHAUST] += pattern.exhaust +
		pattern.punishingDrawExhaust * buffs.normalBuffs[BUFF_PUNISHING_DRAW];
	statuses[STATUS_MOUNT] += pattern.mount;

	fixed = pattern.fixed +
		pattern.shell * buffs.normalBuffs[BUFF_ARTILLERY_MULTIPLIER];

	if (weapon.phial == PHIAL_IMPACT) {
		statuses[STATUS_STUN] += pattern.phialImpactStun;
		statuses[STATUS_EXHAUST] += pattern.phialImpactExhaust;
		double impact_attack_buff = (attack - pre_attack) *
			Constants::instance()->impactPhialRawBonusMultiplier;
		fixed += ((pre_attack * buffs.normalBuffs[BUFF_ARTILLERY_MULTIPLIER]) +
		          impact_attack_buff) * pattern.phialImpactAttack;
	}

	statuses[STATUS_STUN] *= buffs.normalBuffs[BUFF_STUN_MULTIPLIER];
	statuses[STATUS_EXHAUST] *= buffs.normalBuffs[BUFF_EXHAUST_MULTIPLIER];
	statuses[STATUS_MOUNT] *= buffs.normalBuffs[BUFF_MOUNT_MULTIPLIER];
}

DamageData &DamageData::operator=(const DamageData &o) {
	if (&o == this) return *this;
	cut = o.cut;
	impact = o.impact;
	piercing = o.piercing;
	bullet = o.bullet;
	fixed = o.fixed;
	mindsEyeRate = o.mindsEyeRate;
	critRate = o.critRate;
	bounceSharpness = o.bounceSharpness;
	totalRate = o.totalRate;
	std::copy(o.elements, o.elements + ELEMENT_COUNT, elements);
	std::copy(o.statuses, o.statuses + STATUS_COUNT, statuses);
	bounceSharpness = o.bounceSharpness;
	delete buffData;
	if (o.buffData) {
		buffData = new FoldedBuffsData(*o.buffData);
	} else {
		buffData = NULL;
	}
	return *this;
}

void DamageData::setBuffs(const FoldedBuffsData &buffs) {
	delete buffData;
	buffData = new FoldedBuffsData(buffs);
}

void DamageData::clear() {
	cut = 0.0;
	impact = 0.0;
	piercing = 0.0;
	bullet = 0.0;
	fixed = 0.0;
	mindsEyeRate = 0.0;
	critRate = 0.0;
	bounceSharpness.clear();
	totalRate = 0.0;
	std::fill_n(elements, (size_t)ELEMENT_COUNT, 0.0);
	std::fill_n(statuses, (size_t)STATUS_COUNT, 0.0);
	delete buffData;
	buffData = NULL;
}

void DamageData::combineBounceSharpness(const DamageData &o, double rate) {
	if (o.bounceSharpness.isEmpty()) return;
	if (bounceSharpness.isEmpty()) {
		bounceSharpness.resize(o.bounceSharpness.size());
		for (size_t i = 0; i < bounceSharpness.size(); ++i) {
			bounceSharpness[i].rate = o.bounceSharpness[i].rate * rate;
			bounceSharpness[i].multiplier = o.bounceSharpness[i].multiplier;
		}
	} else {
		const BounceSharpnessArray &a =
			bounceSharpness;
		const BounceSharpnessArray &b =
			o.bounceSharpness;
		BounceSharpnessArray n;
		size_t ia = 0;
		size_t ib = 0;
		size_t in = 0;
		while (ia < a.size() || ib < b.size()) {
			if (ib >= b.size() || a[ia].multiplier > b[ib].multiplier) {
				++ia;
			} else if (ia >= a.size() || a[ia].multiplier < b[ib].multiplier) {
				++ib;
			} else {
				++ia, ++ib;
			}
			++in;
		}
		n.resize(in);
		ia = ib = in = 0;
		while (ia < a.size() || ib < b.size()) {
			if (ib >= b.size() || a[ia].multiplier > b[ib].multiplier) {
				n[in++] = a[ia];
				++ia;
			} else if (ia >= a.size() || a[ia].multiplier < b[ib].multiplier) {
				n[in].rate = b[ib].rate * rate;
				n[in++].multiplier = b[ib].multiplier;
				++ib;
			} else {
				n[in].rate = a[ia].rate + b[ib].rate * rate;
				n[in++].multiplier = a[ia].multiplier;
				++ia, ++ib;
			}
		}
		bounceSharpness = n;
	}
}

void DamageData::combine(const DamageData &o, double rate) {
	cut += o.cut * rate;
	impact += o.impact * rate;
	piercing += o.piercing * rate;
	bullet += o.bullet * rate;
	fixed += o.fixed * rate;
	for (int elt = 0; elt < ELEMENT_COUNT; ++elt) {
		elements[elt] += o.elements[elt] * rate;
	}
	for (int sta = 0; sta < STATUS_COUNT; ++sta) {
		statuses[sta] += o.statuses[sta] * rate;
	}
	mindsEyeRate += o.mindsEyeRate * rate;
	critRate += o.critRate * rate;
	combineBounceSharpness(o, rate);
	if (o.buffData) {
		if (!buffData) buffData = new FoldedBuffsData(FoldedBuffsData::ZERO);
		buffData->combine(*o.buffData, rate);
	}
	totalRate += rate;
}

void DamageData::print(QTextStream &stream, QString indent) const {
	stream << indent << "- cut: " << cut << endl;
	stream << indent << "- impact: " << impact << endl;
	stream << indent << "- piercing: " << piercing << endl;
	stream << indent << "- fixed: " << fixed << endl;
	stream << indent << "- elements: [";
	for (int i = 0; i < ELEMENT_COUNT; ++i) {
		if (i > 0) stream << ", ";
		stream << elements[i];
	}
	stream << "]" << endl;
	stream << indent << "- statuses: [";
	for (int i = 0; i < STATUS_COUNT; ++i) {
		if (i > 0) stream << ", ";
		stream << statuses[i];
	}
	stream << "]" << endl;
	stream << indent << "- minds eye rate: " << mindsEyeRate << endl;
	stream << indent << "- bounce sharpness: [";
	for (size_t i = 0; i < bounceSharpness.size(); ++i) {
		if (i > 0) stream << ", ";
		stream << "(" << bounceSharpness[i].rate << ", " <<
			bounceSharpness[i].multiplier << ")";
	}
	stream << "]" << endl;
}
