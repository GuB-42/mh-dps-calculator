#include "Profile.h"
#include "Constants.h"
#include "ConditionRatios.h"

#include <QMap>
#include <QTextStream>
#include <QXmlStreamReader>

Pattern::Pattern() :
	rate(1.0),
	period(1.0),
	cut(0.0),
	impact(0.0),
	piercing(0.0),
	element(0.0),
	status(0.0),
	stun(0.0),
	exhaust(0.0),
	sharpnessMultiplier(1.0),
	phialImpactAttack(0.0),
	phialImpactStun(0.0),
	phialImpactExhaust(0.0),
	phialElementAttack(0.0),
	phialRatio(0.0),
	mindsEyeRatio(0.0),
	sharpnessUse(0.0),
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
	stream << indent << "- element: " << element << endl;
	stream << indent << "- status: " << status << endl;
	stream << indent << "- stun: " << stun << endl;
	stream << indent << "- exhaust: " << exhaust << endl;
	stream << indent << "- sharpness_multiplier: " << sharpnessMultiplier << endl;
	stream << indent << "- phial_impact_attack: " << phialImpactAttack << endl;
	stream << indent << "- phial_impact_stun: " << phialImpactStun << endl;
	stream << indent << "- phial_impact_exhaust: " << phialImpactExhaust << endl;
	stream << indent << "- phial_element_attack: " << phialElementAttack << endl;
	stream << indent << "- phial_ratio: " << phialRatio << endl;
	stream << indent << "- minds_eye_ratio: " << mindsEyeRatio << endl;
	stream << indent << "- sharpness_use: " << sharpnessUse << endl;
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

void Pattern::readXml(QXmlStreamReader *xml, QSet<Condition> *po_cond) {
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
			} else if (tag_name == "element") {
				status = element = xml->readElementText().toDouble();
//			} else if (tag_name == "status") {
//				status = xml->readElementText().toDouble();
			} else if (tag_name == "stun") {
				stun = xml->readElementText().toDouble();
			} else if (tag_name == "exhaust") {
				exhaust = xml->readElementText().toDouble();
			} else if (tag_name == "sharpness_multiplier") {
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
				phialRatio = xml->readElementText().toDouble();
			} else if (tag_name == "minds_eye_ratio") {
				mindsEyeRatio = xml->readElementText().toDouble();
			} else if (tag_name == "sharpness_use") {
				sharpnessUse = xml->readElementText().toDouble();
			} else {
				bool found = false;
				for (int i = 0; i < CONDITION_COUNT; ++i) {
					if (tag_name == QString(toString((Condition)i)) + "_ratio") {
						if (po_cond) po_cond->insert((Condition)i);
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

Profile::Profile() : localRatios(NULL), sharpenPeriod(0.0) {
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
	QMap<Pattern *, QSet<Condition> > om_cond;
	QVector<Pattern *> om_up_cond;

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
			} else if (tag_name == "sharpen_period") {
				sharpenPeriod = xml->readElementText().toDouble();
			} else if (tag_name == "pattern") {
				QSet<Condition> o_cond;
				bool o_sharpen_period = false;
				Pattern *pattern = new Pattern;
				if (localRatios) pattern->conditionRatios = localRatios;
				pattern->readXml(xml, &o_cond);
				if (o_cond.isEmpty()) {
					om_up_cond.append(pattern);
				} else {
					om_cond[pattern] = o_cond;
				}
				patterns.append(pattern);
			} else {
				bool found = false;
				for (int i = 0; i < CONDITION_COUNT; ++i) {
					if (tag_name == QString(toString((Condition)i)) + "_ratio") {
						if (!localRatios) {
							const ConditionRatios *def =
								Constants::instance()->conditionRatios;
							localRatios = new ConditionRatios(*def);
							foreach(Pattern *p, om_up_cond) {
								p->conditionRatios = localRatios;
							}
						}
						double v = xml->readElementText().toDouble();
						(*localRatios)[(Condition)i] = v;
						for (QMap<Pattern *, QSet<Condition> >::const_iterator it =
						     	om_cond.begin();
						     it != om_cond.end(); ++it) {
							if (!it.value().contains((Condition)i)) {
								it.key()->updateConditionRatio((Condition)i, v);
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
}
