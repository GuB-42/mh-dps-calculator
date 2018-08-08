#include "Profile.h"
#include "Constants.h"
#include "ConditionRatios.h"
#include "MotionValue.h"

#include <QDebug>
#include <QMap>
#include <QTextStream>
#include <QXmlStreamReader>

Pattern::Pattern() :
	rate(1.0),
	period(1.0),
	cut(0.0),
	impact(0.0),
	piercing(0.0),
	bullet(0.0),
	fixed(0.0),
	element(0.0),
	status(0.0),
	stun(0.0),
	exhaust(0.0),
	mount(0.0),
	sharpnessMultiplier(1.0),
	phialImpactAttack(0.0),
	phialImpactStun(0.0),
	phialImpactExhaust(0.0),
	phialElementAttack(0.0),
	phialRatio(0.0),
	mindsEyeRatio(0.0),
	sharpnessUse(0.0),
	punishingDrawStun(0.0),
	punishingDrawExhaust(0.0),
	definedPhialRatio(false),
	definedMindsEyeRatio(false),
	definedSharpnessMultiplier(false),
	conditionRatios(Constants::instance()->conditionRatios),
	localRatios(NULL)
{
}

Pattern::~Pattern() {
	delete localRatios;
}

void Pattern::print(QTextStream &stream, QString indent) const {
	stream << indent << "- rate: " << rate << endl;
	stream << indent << "- period: " << period << endl;
	stream << indent << "- cut: " << cut << endl;
	stream << indent << "- impact: " << impact << endl;
	stream << indent << "- piercing: " << piercing << endl;
	stream << indent << "- bullet: " << bullet << endl;
	stream << indent << "- fixed: " << fixed << endl;
	stream << indent << "- element: " << element << endl;
	stream << indent << "- status: " << status << endl;
	stream << indent << "- stun: " << stun << endl;
	stream << indent << "- exhaust: " << exhaust << endl;
	stream << indent << "- mount: " << mount << endl;
	stream << indent << "- sharpness_multiplier: " << sharpnessMultiplier << endl;
	stream << indent << "- phial_impact_attack: " << phialImpactAttack << endl;
	stream << indent << "- phial_impact_stun: " << phialImpactStun << endl;
	stream << indent << "- phial_impact_exhaust: " << phialImpactExhaust << endl;
	stream << indent << "- phial_element_attack: " << phialElementAttack << endl;
	stream << indent << "- phial_ratio: " << phialRatio << endl;
	stream << indent << "- minds_eye_ratio: " << mindsEyeRatio << endl;
	stream << indent << "- sharpness_use: " << sharpnessUse << endl;
	stream << indent << "- punishing_draw_stun: " << punishingDrawStun << endl;
	stream << indent << "- punishing_draw_exhaust: " << punishingDrawExhaust << endl;
	foreach(const MotionValueRef &mvr, motionValueRefs) {
		stream << indent << "- motion value: " << mvr.id <<
			" (multiplier: " << mvr.multiplier << ") (raw multiplier: " <<
			mvr.rawMultiplier << ")" << endl;
	}
	stream << indent << "- condition ratios";
	if (localRatios) stream << " *";
	stream << endl;
	conditionRatios->print(stream, indent + "\t");
}

void Pattern::updateConditionRatio(Condition cond, double v) {
	if (!localRatios) localRatios = new ConditionRatios(*conditionRatios);
	conditionRatios = localRatios;
	(*localRatios)[cond] = v;
}

void Pattern::applyMotionValue(const MotionValue *mv,
                               double multiplier, double raw_multiplier,
                               const MultiplierTotals &totals) {
	cut += mv->cut * multiplier * raw_multiplier;
	impact += mv->impact * multiplier * raw_multiplier;
	piercing += mv->piercing * multiplier * raw_multiplier;
	bullet += mv->bullet * multiplier * raw_multiplier;
	fixed += mv->fixed * multiplier;
	element += mv->element * multiplier;
	status += mv->status * multiplier;
	stun += mv->stun * multiplier;
	exhaust += mv->exhaust * multiplier;
	mount += mv->mount * multiplier;
	phialImpactAttack += mv->phialImpactAttack * multiplier;
	phialImpactStun += mv->phialImpactStun * multiplier;
	phialImpactExhaust += mv->phialImpactExhaust * multiplier;
	phialElementAttack += mv->phialElementAttack * multiplier;
	sharpnessUse += mv->sharpnessUse * multiplier;
	punishingDrawStun += mv->punishingDrawStun * multiplier;
	punishingDrawExhaust += mv->punishingDrawExhaust * multiplier;
	if (totals.sharpnessMultiplier > 0.0) {
		double weight = multiplier / totals.sharpnessMultiplier;
		sharpnessMultiplier += mv->sharpnessMultiplier * weight;
	}
	if (totals.drawAttackRatio > 0.0) {
		double weight = multiplier / totals.drawAttackRatio;
		double cr = (*conditionRatios)[CONDITION_DRAW_ATTACK];
		updateConditionRatio(CONDITION_DRAW_ATTACK,
		                     cr + mv->drawAttackRatio * weight);
	}
	if (totals.airborneRatio > 0.0) {
		double weight = multiplier / totals.airborneRatio;
		double cr = (*conditionRatios)[CONDITION_AIRBORNE];
		updateConditionRatio(CONDITION_AIRBORNE,
		                     cr + mv->airborneRatio * weight);
	}
	if (totals.phialRatio > 0.0) {
		double weight = multiplier / totals.phialRatio;
		phialRatio += mv->phialRatio * weight;
	}
	if (totals.mindsEyeRatio > 0.0) {
		double weight = multiplier / totals.mindsEyeRatio;
		mindsEyeRatio += mv->mindsEyeRatio * weight;
	}
}

void Pattern::applyMotionValues(const QHash<QString, MotionValue *> &mv_hash) {
	MultiplierTotals totals;
	foreach(const MotionValueRef &mvr, motionValueRefs) {
		QHash<QString, MotionValue *>::const_iterator it =
			mv_hash.find(mvr.id);
		if (it != mv_hash.end()) {
			const MotionValue *mv = *it;
			if (!definedSharpnessMultiplier &&
			    mv->definedSharpnessMultiplier) {
				totals.sharpnessMultiplier += mvr.multiplier;
			}
			if (!definedConditionRatios[CONDITION_DRAW_ATTACK] &&
			    mv->definedDrawAttackRatio) {
				totals.drawAttackRatio += mvr.multiplier;
			}
			if (!definedConditionRatios[CONDITION_AIRBORNE] &&
			    mv->definedAirborneRatio) {
				totals.airborneRatio += mvr.multiplier;
			}
			if (!definedPhialRatio && mv->definedPhialRatio) {
				totals.phialRatio += mvr.multiplier;
			}
			if (!definedMindsEyeRatio && mv->definedMindsEyeRatio) {
				totals.mindsEyeRatio += mvr.multiplier;
			}
		}
	}
	foreach(const MotionValueRef &mvr, motionValueRefs) {
		QHash<QString, MotionValue *>::const_iterator it =
			mv_hash.find(mvr.id);
		if (it != mv_hash.end()) {
			applyMotionValue(*it, mvr.multiplier, mvr.rawMultiplier, totals);
		} else {
			qWarning() << "Unknown motion value \"" << mvr.id << "\"";
		}
	}
}

void Pattern::readXml(QXmlStreamReader *xml) {
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			QStringRef tag_name = xml->name();
			if (tag_name == "rate") {
				rate = xml->readElementText().toDouble();
			} else if (tag_name == "period") {
				period = xml->readElementText().toDouble();
			} else if (tag_name == "cut") {
				cut = xml->readElementText().toDouble();
			} else if (tag_name == "impact") {
				impact = xml->readElementText().toDouble();
			} else if (tag_name == "piercing") {
				piercing = xml->readElementText().toDouble();
			} else if (tag_name == "bullet") {
				bullet = xml->readElementText().toDouble();
			} else if (tag_name == "fixed") {
				fixed = xml->readElementText().toDouble();
			} else if (tag_name == "element") {
				status = element = xml->readElementText().toDouble();
//			} else if (tag_name == "status") {
//				status = xml->readElementText().toDouble();
			} else if (tag_name == "stun") {
				stun = xml->readElementText().toDouble();
			} else if (tag_name == "exhaust") {
				exhaust = xml->readElementText().toDouble();
			} else if (tag_name == "mount") {
				mount = xml->readElementText().toDouble();
			} else if (tag_name == "sharpness_multiplier") {
				definedSharpnessMultiplier = true;
				sharpnessMultiplier = xml->readElementText().toDouble();
			} else if (tag_name == "phial_impact_attack") {
				phialImpactAttack = xml->readElementText().toDouble();
			} else if (tag_name == "phial_impact_stun") {
				phialImpactStun = xml->readElementText().toDouble();
			} else if (tag_name == "phial_impact_exhaust") {
				phialImpactExhaust = xml->readElementText().toDouble();
			} else if (tag_name == "phial_element_attack") {
				phialElementAttack = xml->readElementText().toDouble();
			} else if (tag_name == "phial_ratio") {
				definedPhialRatio = true;
				phialRatio = xml->readElementText().toDouble();
			} else if (tag_name == "minds_eye_ratio") {
				definedMindsEyeRatio = true;
				mindsEyeRatio = xml->readElementText().toDouble();
			} else if (tag_name == "sharpness_use") {
				sharpnessUse = xml->readElementText().toDouble();
			} else if (tag_name == "motion_value_ref") {
				MotionValueRef mvr;
				mvr.id = xml->attributes().value("id").toString();
				if (xml->attributes().hasAttribute("multiplier")) {
					mvr.multiplier =
						xml->attributes().value("multiplier").
						toString().toDouble();
				}
				if (xml->attributes().hasAttribute("raw_multiplier")) {
					mvr.rawMultiplier =
						xml->attributes().value("raw_multiplier").
						toString().toDouble();
				}
				motionValueRefs.append(mvr);
				xml->skipCurrentElement();
			} else {
				bool found = false;
				for (int i = 0; i < CONDITION_COUNT; ++i) {
					if (tag_name == QString(toString((Condition)i)) + "_ratio") {
						definedConditionRatios[i] = true;
						updateConditionRatio((Condition)i, xml->readElementText().toDouble());
						found = true;
						break;
					}
				}
				if (!found) xml->skipCurrentElement();
			}
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
}

Profile::Profile() : sharpenPeriod(0.0), localRatios(NULL) {
}

Profile::~Profile() {
	delete localRatios;
	foreach(Pattern *pattern, patterns) delete pattern;
}

void Profile::print(QTextStream &stream, QString indent) const {
	NamedObject::print(stream, indent);
	stream << indent << "- type: " << type << endl;
	stream << indent << "- sharpen_period: " << sharpenPeriod << endl;
	foreach(const Pattern *pattern, patterns) {
		stream << indent << "- pattern" << endl;
		pattern->print(stream, indent + "\t");
	}
}

void Profile::readXml(QXmlStreamReader *xml) {
	double glob_rate = 1.0;
	sharpenPeriod = Constants::instance()->sharpenPeriod;

	if (xml->attributes().hasAttribute("id")) {
		id = xml->attributes().value("id").toString();
	}
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			QStringRef tag_name = xml->name();
			if (readXmlName(xml)) {
				; // name
			} else if (tag_name == "type") {
				type = xml->readElementText();
			} else if (tag_name == "rate") {
				glob_rate = xml->readElementText().toDouble();
			} else if (tag_name == "sharpen_period") {
				sharpenPeriod = xml->readElementText().toDouble();
			} else if (tag_name == "pattern") {
				Pattern *pattern = new Pattern;
				if (localRatios) pattern->conditionRatios = localRatios;
				pattern->readXml(xml);
				patterns.append(pattern);
			} else {
				bool found = false;
				for (int i = 0; i < CONDITION_COUNT; ++i) {
					if (tag_name == QString(toString((Condition)i)) + "_ratio") {
						if (!localRatios) {
							const ConditionRatios *def =
								Constants::instance()->conditionRatios;
							localRatios = new ConditionRatios(*def);
							foreach(Pattern *pattern, patterns) {
								if (pattern->definedConditionRatios.none()) {
									pattern->conditionRatios = localRatios;
								}
							}
						}
						double v = xml->readElementText().toDouble();
						(*localRatios)[(Condition)i] = v;
						foreach(Pattern *pattern, patterns) {
							if (!pattern->definedConditionRatios[i]) {
								pattern->updateConditionRatio((Condition)i, v);
							}
						}
						found = true;
						break;
					}
				}
				if (!found) xml->skipCurrentElement();
			}
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
	foreach(Pattern *pattern, patterns) {
		pattern->rate *= glob_rate;
	}
}
