#include "Dps.h"

#include <QTextStream>
#include "Monster.h"
#include "Target.h"
#include "DamageData.h"
#include "Damage.h"
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
	double hits = 0.0;
	double time = 0.0;
	double tol = tolerance.initial;
	bool first = true;
	while (1) {
		if (tol > tolerance.max) tol = tolerance.max;
		double dt = tol / status_attack;
		if (!first) {
			if (overbuild) {
				if (dt < tolerance.duration) dt = tolerance.duration;
			} else {
				dt += tolerance.duration;
			}
		}
		if (time + dt <= period && (first || tol != tolerance.max)) {
			hits += 1.0;
			time += dt;
			tol += tolerance.plus;
		} else {
			hits += (period - time) / dt;
			break;
		}
		first = false;
	}
	return hits / period;
}

void Dps::computeNoStatus(const MonsterHitData &hit_data,
                          const DamageData &normal_damage,
                          const DamageData &weak_damage) {
	double t_atk_normal[4] = {
		normal_damage.cut, normal_damage.impact,
		normal_damage.piercing, normal_damage.bullet
	};
	double t_atk_weak[4] = {
		weak_damage.cut, weak_damage.impact,
		weak_damage.piercing, weak_damage.bullet
	};
	double t_def[4] = {
		hit_data.cut, hit_data.impact,
		hit_data.impact * Constants::instance()->piercingFactor,
		hit_data.bullet
	};
	if (t_def[2] < t_def[0]) t_def[2] = t_def[0];

	raw = 0.0;
	bounceRate = 0.0;
	critRate = 0.0;
	double bounce_divider = 0.0;
	double crit_divider = 0.0;
	double minds_eye = 0.0;
	double weak_raw = 0.0;
	for (int i = 0; i < 4; ++i) {
		bool weak = t_def[i] > Constants::instance()->rawWeakSpotThreshold;
		const double (&t_atk)[4] = weak ? t_atk_weak : t_atk_normal;
		if (t_atk[i] == 0.0) continue;
		const DamageData &dmg = weak ? weak_damage : normal_damage;
		double t_raw = t_atk[i] * t_def[i] / 100.0;
		raw += t_raw;
		if (weak) weak_raw += t_atk[i] * t_def[i] / 100.0;
		if (i != 3) {
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
		critRate += dmg.critRate * t_atk[i];
		for (int j = 0; j < 4; ++j) {
			crit_divider += t_atk[i] * t_atk[j];
		}
	}
	if (bounce_divider > 0.0) {
		bounceRate *= 1.0 - minds_eye / bounce_divider;
		bounceRate /= bounce_divider;
	}
	if (bounceRate < 1e-9) bounceRate = 0.0;
	if (crit_divider) critRate /= crit_divider;

	fixed = weak_damage.fixed;

	totalElements = 0.0;
	for (int elt = 0; elt < ELEMENT_COUNT; ++elt) {
		bool weak = hit_data.element[elt] <
			Constants::instance()->elementWeakSpotThreshold;
		const DamageData &dmg = weak ? normal_damage : weak_damage;
		double v = dmg.elements[elt] * hit_data.element[elt] / 100.0;
		elements[elt] = v;
		totalElements += v;
	}

	stunRate = hit_data.stun / 100.0;
	weakSpotRatio = (raw != 0.0 ? weak_raw / raw : 0.0);
}

void Dps::computeStatus(const Monster &monster,
                        const DamageData &normal_damage,
                        const DamageData &weak_damage,
                        double defense_multiplier,
                        double status_defense_multiplier,
                        double status_hit_multiplier) {
	killFrequency = 0.0;
	totalStatuses = 0.0;
	for (int sta = 0; sta < STATUS_COUNT; ++sta) {
		statusProcRate[sta] = 0.0;
	}
	double subtotal = (raw + fixed + totalElements) * defense_multiplier;
	for (int retry = 0; retry < 3; ++retry) {
		double real_total =
			subtotal + totalStatuses * status_defense_multiplier;
		totalStatuses = 0.0;
		if (monster.hitPoints > 0.0	&& real_total > 0.0) {
			killFrequency = real_total / monster.hitPoints;
			for (int sta = 0; sta < STATUS_COUNT; ++sta) {
				if (monster.tolerances[sta]) {
					double status_attack =
						normal_damage.statuses[sta] * (1.0 - weakSpotRatio) +
						weak_damage.statuses[sta] * weakSpotRatio;
					if (sta == STATUS_STUN) status_attack *= stunRate;
					double hits = get_status_hits(status_attack,
					                              *monster.tolerances[sta],
					                              1.0 / killFrequency,
					                              sta == STATUS_POISON);
					if (hits > 0.0) {
						hits *= status_hit_multiplier;
						statusProcRate[sta] = hits;
						if (monster.tolerances[sta]->damage > 0.0) {
							double mod_hits = killFrequency *
								 Constants::statusDamageCurve(hits / killFrequency);
							double v = monster.tolerances[sta]->damage * mod_hits;
							statuses[sta] = v;
							totalStatuses += v;
						}
					}
				}
			}
		}
		if (totalStatuses == 0.0) break;
	}
}

void Dps::compute(const Target &target, const Damage &damage)
{
	bool enraged_equals_normal =
		damage.data[MODE_NORMAL_NORMAL] ==
		damage.data[MODE_ENRAGED_NORMAL] &&
		damage.data[MODE_NORMAL_WEAK_SPOT] ==
		damage.data[MODE_ENRAGED_WEAK_SPOT];

	Dps t_dps;
	foreach(TargetMonster *tmonster, target.targetMonsters) {
		if (enraged_equals_normal) {
			double monster_weight = 0.0;
			foreach(TargetZone *tzone, tmonster->targetZones) {
				monster_weight += tzone->weight;
			}
			Dps monster_dps;
			foreach(TargetZone *tzone, tmonster->targetZones) {
				t_dps.computeNoStatus(*tzone->hitData,
				                      *damage.data[MODE_NORMAL_NORMAL],
				                      *damage.data[MODE_NORMAL_WEAK_SPOT]);
				monster_dps.combine(t_dps, tzone->weight / monster_weight);
			}
			monster_dps.computeStatus(*tmonster->monster,
			                          *damage.data[MODE_NORMAL_NORMAL],
			                          *damage.data[MODE_NORMAL_WEAK_SPOT],
			                          tmonster->defenseMultiplier,
			                          tmonster->statusDefenseMultiplier,
			                          tmonster->statusHitMultiplier);
			combine(monster_dps, monster_weight);
		} else {
			double normal_weight = 0.0;
			double enraged_weight = 0.0;
			foreach(TargetZone *tzone, tmonster->targetZones) {
				normal_weight += tzone->weight * (1.0 - tzone->enragedRatio);
				enraged_weight += tzone->weight * tzone->enragedRatio;
			}
			Dps normal_dps;
			Dps enraged_dps;
			foreach(TargetZone *tzone, tmonster->targetZones) {
				t_dps.computeNoStatus(*tzone->hitData,
				                      *damage.data[MODE_NORMAL_NORMAL],
				                      *damage.data[MODE_NORMAL_WEAK_SPOT]);
				normal_dps.combine(t_dps, tzone->weight * (1.0 - tzone->enragedRatio) /
				                   normal_weight);
				t_dps.computeNoStatus(*tzone->hitData,
				                      *damage.data[MODE_ENRAGED_NORMAL],
				                      *damage.data[MODE_ENRAGED_WEAK_SPOT]);
				enraged_dps.combine(t_dps, tzone->weight * tzone->enragedRatio /
				                    enraged_weight);
			}
			normal_dps.computeStatus(*tmonster->monster,
			                         *damage.data[MODE_NORMAL_NORMAL],
			                         *damage.data[MODE_NORMAL_WEAK_SPOT],
			                         tmonster->defenseMultiplier,
			                         tmonster->statusDefenseMultiplier,
			                         tmonster->statusHitMultiplier);
			combine(normal_dps, normal_weight);
			enraged_dps.computeStatus(*tmonster->monster,
			                          *damage.data[MODE_ENRAGED_NORMAL],
			                          *damage.data[MODE_ENRAGED_WEAK_SPOT],
			                          tmonster->defenseMultiplier,
			                          tmonster->statusDefenseMultiplier,
			                          tmonster->statusHitMultiplier);
			combine(enraged_dps, enraged_weight);
		}
	}
}

Dps::Dps() :
	raw(0.0), totalElements(0.0), totalStatuses(0.0), fixed(0.0),
	bounceRate(0.0), critRate(0.0), killFrequency(0.0),
	weakSpotRatio(0.0), stunRate(0.0)
{
	std::fill_n(elements, ELEMENT_COUNT, 0.0);
	std::fill_n(statuses, STATUS_COUNT, 0.0);
	std::fill_n(statusProcRate, STATUS_COUNT, 0.0);
}

void Dps::combine(const Dps &o, double rate) {
	raw += o.raw * rate;
	fixed += o.fixed * rate;
	bounceRate += o.bounceRate * rate;
	critRate += o.critRate * rate;
	killFrequency += o.killFrequency * rate;
	weakSpotRatio += o.weakSpotRatio * rate;
	stunRate += o.stunRate * rate;
	if (o.totalElements != 0.0) {
		totalElements += o.totalElements * rate;
		for (int i = 0; i < ELEMENT_COUNT; ++i) {
			elements[i] += o.elements[i] * rate;
		}
	}
	if (o.totalStatuses != 0.0) {
		totalStatuses += o.totalStatuses * rate;
		for (int i = 0; i < STATUS_COUNT; ++i) {
			statuses[i] += o.statuses[i] * rate;
		}
	}
	for (int i = 0; i < STATUS_COUNT; ++i) {
		statusProcRate[i] += o.statusProcRate[i] * rate;
	}
}

void Dps::print(QTextStream &stream, QString indent) {
	stream << indent << "- raw: " << raw << endl;
	stream << indent << "- total elements: " << totalElements << endl;
	stream << indent << "- elements: [";
	for (int i = 0; i < ELEMENT_COUNT; ++i) {
		if (i > 0) stream << ", ";
		stream << elements[i];
	}
	stream << "]" << endl;
	stream << indent << "- total statuses: " << totalStatuses << endl;
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
