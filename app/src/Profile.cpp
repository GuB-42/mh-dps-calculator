#include "Profile.h"
#include "Constants.h"
#include "ConditionRatios.h"
#include "MotionValue.h"
#include "WeaponType.h"
#include "Ammo.h"

#include <QDebug>
#include <QMap>
#include <QTextStream>
#include <QXmlStreamReader>
#include "QtCompatibility.h"

Pattern::Pattern() :
	rate(1.0),
	period(1.0),
	usage(0.0),
	capacityUpFilter(false),
	capacityUpEnabled(false),
	cut(0.0),
	impact(0.0),
	piercing(0.0),
	bullet(0.0),
	shell(0.0),
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
	XmlObject::print(stream, indent);
	stream << indent << "- rate: " << rate << ENDL;
	stream << indent << "- period: " << period << ENDL;
	stream << indent << "- capacityUpFilter: " <<
		(capacityUpFilter ? "true" : "false") << ENDL;
	stream << indent << "- capacityUpEnabled: " <<
		(capacityUpEnabled ? "true" : "false") << ENDL;
	stream << indent << "- ammos: [";
	for (int i = 0; i < ammoRefs.count(); ++i) {
		if (i > 0) stream << ", ";
		if (ammoRefs[i].ammo) {
			stream << ammoRefs[i].ammo->id;
		} else {
			stream << "<null, " << ammoRefs[i].id << ">";
		}
	}
	stream << "]" << ENDL;
	stream << indent << "- cut: " << cut << ENDL;
	stream << indent << "- impact: " << impact << ENDL;
	stream << indent << "- piercing: " << piercing << ENDL;
	stream << indent << "- bullet: " << bullet << ENDL;
	stream << indent << "- shell: " << shell << ENDL;
	stream << indent << "- fixed: " << fixed << ENDL;
	stream << indent << "- element: " << element << ENDL;
	stream << indent << "- status: " << status << ENDL;
	stream << indent << "- stun: " << stun << ENDL;
	stream << indent << "- exhaust: " << exhaust << ENDL;
	stream << indent << "- mount: " << mount << ENDL;
	stream << indent << "- sharpness_multiplier: " << sharpnessMultiplier << ENDL;
	stream << indent << "- phial_impact_attack: " << phialImpactAttack << ENDL;
	stream << indent << "- phial_impact_stun: " << phialImpactStun << ENDL;
	stream << indent << "- phial_impact_exhaust: " << phialImpactExhaust << ENDL;
	stream << indent << "- phial_element_attack: " << phialElementAttack << ENDL;
	stream << indent << "- phial_ratio: " << phialRatio << ENDL;
	stream << indent << "- minds_eye_ratio: " << mindsEyeRatio << ENDL;
	stream << indent << "- sharpness_use: " << sharpnessUse << ENDL;
	stream << indent << "- punishing_draw_stun: " << punishingDrawStun << ENDL;
	stream << indent << "- punishing_draw_exhaust: " << punishingDrawExhaust << ENDL;
	foreach(const MotionValueRef &mvr, motionValueRefs) {
		stream << indent << "- motion value: " << mvr.id <<
			" (multiplier: " << mvr.multiplier << ") (raw multiplier: " <<
			mvr.rawMultiplier << ")" << ENDL;
	}
	stream << indent << "- condition ratios";
	if (localRatios) stream << " *";
	stream << ENDL;
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
	shell += mv->shell * multiplier;
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

bool Pattern::readXmlElement(QXmlStreamReader *xml) {
	if (XmlObject::readXmlElement(xml)) return true;
	QStringRef tag_name = xml->name();
	if (tag_name == "rate") {
		rate = xml->readElementText().toDouble();
	} else if (tag_name == "period") {
		period = xml->readElementText().toDouble();
	} else if (tag_name == "usage") {
		usage = xml->readElementText().toDouble();
	} else if (tag_name == "capacity_up") {
		QString v = xml->readElementText().toLower().trimmed();
		capacityUpFilter = true;
		capacityUpEnabled =
			(v != "false" || v != "0" || v != "no" || v != "disabled");
	} else if (tag_name == "ammo_ref") {
		PatternAmmoRef ammo_ref;
		ammo_ref.id = xml->attributes().value("id").toString();
		ammoRefs.append(ammo_ref);
		XML_SKIP_CURRENT_ELEMENT(*xml);
	} else if (tag_name == "cut") {
		cut = xml->readElementText().toDouble();
	} else if (tag_name == "impact") {
		impact = xml->readElementText().toDouble();
	} else if (tag_name == "piercing") {
		piercing = xml->readElementText().toDouble();
	} else if (tag_name == "bullet") {
		bullet = xml->readElementText().toDouble();
	} else if (tag_name == "shell") {
		shell = xml->readElementText().toDouble();
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
		XML_SKIP_CURRENT_ELEMENT(*xml);
	} else {
		for (int i = 0; i < CONDITION_COUNT; ++i) {
			if (tag_name == QString(toString((Condition)i)) + "_ratio") {
				definedConditionRatios[i] = true;
				updateConditionRatio((Condition)i, xml->readElementText().toDouble());
				return true;
			}
		}
		return false;
	}
	return true;
}

Profile::Profile() :
	weaponType(NULL), rate(1.0), localRatios(NULL)
{
	sharpenPeriod = Constants::instance()->sharpenPeriod;
}

Profile::~Profile() {
	delete localRatios;
	foreach(Pattern *pattern, patterns) delete pattern;
}

void Profile::print(QTextStream &stream, QString indent) const {
	NamedObject::print(stream, indent);
	stream << indent << "- weapon_type: ";
	if (weaponType) {
		stream << weaponType->id << ENDL;
	} else {
		stream << "<null, " << weaponTypeRefId << ">" << ENDL;
	}
	stream << indent << "- sharpen_period: " << sharpenPeriod << ENDL;
	foreach(const Pattern *pattern, patterns) {
		stream << indent << "- pattern" << ENDL;
		pattern->print(stream, indent + "\t");
	}
}

bool Profile::readXmlElement(QXmlStreamReader *xml) {
	if (NamedObject::readXmlElement(xml)) return true;
	QStringRef tag_name = xml->name();
	if (tag_name == "weapon_type_ref") {
		weaponTypeRefId = xml->attributes().value("id").toString();
		XML_SKIP_CURRENT_ELEMENT(*xml);
	} else if (tag_name == "rate") {
		rate = xml->readElementText().toDouble();
	} else if (tag_name == "sharpen_period") {
		sharpenPeriod = xml->readElementText().toDouble();
	} else if (tag_name == "pattern") {
		Pattern *pattern = new Pattern;
		if (localRatios) pattern->conditionRatios = localRatios;
		pattern->readXml(xml);
		patterns.append(pattern);
	} else {
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
				return true;
			}
		}
		return false;
	}
	return true;
}

void Profile::readXml(QXmlStreamReader *xml) {
	NamedObject::readXml(xml);
	foreach(Pattern *pattern, patterns) {
		pattern->rate *= rate;
	}
}
