#include "Dps.h"

#include <QTextStream>
#include "Monster.h"
#include "DamageData.h"
#include "Constants.h"

double get_status_hits(double status_attack,
                       const MonsterTolerance &tolerance,
                       double period,
                       bool overbuild)
{
	if (tolerance.regenValue > 0.0) {
		status_attack -=
			tolerance.regenValue / tolerance.regenTick;
	}
	if (status_attack <= 0.0) return 0.0;
	double duration = 0.0;
	double hits = 0.0;
	for (int retry = 0; retry < 3; ++retry) {
		hits = 0.0;
		double dmg = status_attack * (period - duration);
		double tol = tolerance.initial;
		while (tol < tolerance.max && dmg > 0.0) {
			if (dmg > tol) {
				dmg -= tol;
				tol += tolerance.plus;
				hits += 1.0;
			} else {
				hits += dmg / tol;
				dmg = 0.0;
			}
		}
		if (tolerance.max > 0.0) hits += dmg / tolerance.max;
		duration = hits * tolerance.duration;
		if (overbuild || duration == 0.0) break;
	}
	return hits;
}

Dps::Dps(const Monster &monster,
         const MonsterHitData &hit_data,
         const DamageData &normal_damage,
         const DamageData &weak_damage,
         double defense_multiplier) {
	double t_atk_normal[3] = {
		normal_damage.cut, normal_damage.impact, normal_damage.piercing
	};
	double t_atk_weak[3] = {
		weak_damage.cut, weak_damage.impact, weak_damage.piercing
	};
	double t_def[3] = {
		hit_data.cut, hit_data.impact,
		hit_data.impact * Constants::instance()->piercingFactor
	};
	if (t_def[2] < t_def[0]) t_def[2] = t_def[0];

	raw = 0.0;
	bounceRate = 0.0;
	double bounce_divider = 0.0;
	double minds_eye = 0.0;
	for (int i = 0; i < 3; ++i) {
		bool weak = t_def[i] < Constants::instance()->rawWeakSpotThreshold;
		const DamageData &dmg = weak ? normal_damage : weak_damage;
		const double (&t_atk)[3] = weak ? t_atk_normal : t_atk_weak;
		raw += t_atk[i] * t_def[i] / 100.0;
		foreach(const SharpnessMultiplierData smd, dmg.bounceSharpness) {
			double v = t_atk[i] * smd.rate;
			bounce_divider += v;
			if (smd.multiplier * t_def[i] <
			    Constants::instance()->bounceThreshold) {
				bounceRate += v;
			}
		}
		minds_eye += dmg.mindsEyeRate * t_atk[i];
	}
	if (bounce_divider > 0.0) {
		bounceRate *= 1.0 - minds_eye / bounce_divider;
		bounceRate /= bounce_divider;
	}

	fixed = weak_damage.fixed;

	total_elements = 0.0;
	for (int elt = 0; elt < ELEMENT_COUNT; ++elt) {
		bool weak = hit_data.element[elt] <
			Constants::instance()->elementWeakSpotThreshold;
		const DamageData &dmg = weak ? normal_damage : weak_damage;
		double v = dmg.elements[elt] * hit_data.element[elt] / 100.0;
		elements[elt] = v;
		total_elements += v;
	}

	killFrequency = 0.0;
	total_statuses = 0.0;
	for (int sta = 0; sta < STATUS_COUNT; ++sta) {
		statusProcRate[sta] = 0.0;
	}
	for (int retry = 0; retry < 3; ++retry) {
		total = raw + fixed + total_statuses + total_elements;
		double real_total = total * defense_multiplier;
		total_statuses = 0.0;
		if (monster.hitPoints > 0.0	&& real_total > 0.0) {
			killFrequency = real_total / monster.hitPoints;
			for (int sta = 0; sta < STATUS_COUNT; ++sta) {
				if (monster.tolerances[sta]) {
					double status_attack = weak_damage.statuses[sta];
					if (sta == STATUS_STUN) {
						status_attack *= hit_data.stun / 100.0;
					}
					double hits = get_status_hits(status_attack,
					                              *monster.tolerances[sta],
					                              1.0 / killFrequency,
					                              sta == STATUS_POISON);
					if (hits > 0.0) {
						double v = hits * monster.tolerances[sta]->damage;
						statuses[sta] = v;
						statusProcRate[sta] = hits;
						total_statuses += v;
					}
				}
			}
		}
		if (total_statuses == 0.0) break;
	}
}

Dps::Dps() :
	total(0.0), raw(0.0), total_elements(0.0), total_statuses(0.0), fixed(0.0),
	bounceRate(0.0), killFrequency(0.0)
{
	for (int i = 0; i < ELEMENT_COUNT; ++i) {
		elements[i] = 0.0;
	}
	for (int i = 0; i < STATUS_COUNT; ++i) {
		statuses[i] = 0.0;
		statusProcRate[i] = 0.0;
	}
}

void Dps::combine(const Dps &o, double rate) {
	total += o.total * rate;
	raw += o.raw * rate;
	total_elements += o.total_elements * rate;
	total_statuses += o.total_statuses * rate;
	fixed += o.fixed * rate;
	bounceRate += o.bounceRate * rate;
	killFrequency += o.killFrequency * rate;
	for (int i = 0; i < ELEMENT_COUNT; ++i) {
		elements[i] += o.elements[i] * rate;;
	}
	for (int i = 0; i < STATUS_COUNT; ++i) {
		statuses[i] += o.statuses[i] * rate;
		statusProcRate[i] = o.statusProcRate[i] * rate;
	}
}

void Dps::print(QTextStream &stream, QString indent) {
	stream << indent << "- raw: " << raw << endl;
	stream << indent << "- total elements: " << total_elements << endl;
	stream << indent << "- elements: [";
	for (int i = 0; i < ELEMENT_COUNT; ++i) {
		if (i > 0) stream << ", ";
		stream << elements[i];
	}
	stream << "]" << endl;
	stream << indent << "- total statuses: " << total_statuses << endl;
	stream << indent << "- statuses: [";
	for (int i = 0; i < STATUS_COUNT; ++i) {
		if (i > 0) stream << ", ";
		stream << statuses[i];
	}
	stream << "]" << endl;
	stream << indent << "- fixed: " << fixed << endl;
	stream << indent << "- bounce rate: " << bounceRate << endl;
	stream << indent << "- status proc rate: [";
	for (int i = 0; i < STATUS_COUNT; ++i) {
		if (i > 0) stream << ", ";
		stream << statusProcRate[i];
	}
	stream << "]" << endl;
	stream << indent << "- kill frequency: " << killFrequency << endl;
}
