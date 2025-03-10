#include "Monster.h"

#include <QTextStream>
#include <QXmlStreamReader>
#include "QtCompatibility.h"

MonsterHitData::MonsterHitData() :
	enragedState(0.5), cut(0.0), impact(0.0), bullet(0.0), stun(0.0)
{
	std::fill_n(element, (size_t)ELEMENT_COUNT, 0.0);
}

void MonsterHitData::print(QTextStream &stream, QString indent) const {
	XmlObject::print(stream, indent);
	stream << indent << "(" << state << "/" << enragedState << ") " <<
	"[" << cut << ", " << impact << ", " << bullet << ", ";
	for (int i = 0; i < ELEMENT_COUNT; ++i) {
		stream << element[i] << ", ";

	}
	stream << stun << "]" << ENDL;
}

bool MonsterHitData::readXmlElement(QXmlStreamReader *xml) {
	if (XmlObject::readXmlElement(xml)) return true;
	QStringRef tag_name = xml->name();
	if (tag_name == "state") {
		state = xml->readElementText();
	} else if (tag_name == "enraged_state") {
		enragedState = xml->readElementText().toDouble();
	} else if (tag_name == "cut") {
		cut = xml->readElementText().toDouble();
	} else if (tag_name == "impact") {
		impact = xml->readElementText().toDouble();
	} else if (tag_name == "bullet") {
		bullet = xml->readElementText().toDouble();
	} else if (tag_name == "stun") {
		stun = xml->readElementText().toDouble();
	} else {
		for (int i = 0; i < ELEMENT_COUNT; ++i) {
			if (tag_name == toString((ElementType)i)) {
				element[i] = xml->readElementText().toDouble();
				return true;
			}
		}
		return false;
	}
	return true;
}

MonsterPart::~MonsterPart() {
	foreach(MonsterHitData *h, hitData) delete h;
}

void MonsterPart::print(QTextStream &stream, QString indent) const {
	NamedObject::print(stream, indent);
	foreach(const MonsterHitData *hit_data, hitData) {
		stream << indent << "- hit data" << ENDL;
		hit_data->print(stream, indent + "\t");
	}
}

bool MonsterPart::readXmlElement(QXmlStreamReader *xml) {
	if (NamedObject::readXmlElement(xml)) return true;
	QStringRef tag_name = xml->name();
	if (tag_name == "hit_data") {
		MonsterHitData *hit_data = new MonsterHitData;
		hit_data->readXml(xml);
		hitData.append(hit_data);
	} else {
		return false;
	}
	return true;
}

MonsterTolerance::MonsterTolerance() :
	initial(0.0), plus(0.0), max(0.0), regenValue(0.0),
	regenTick(0.0), duration(0.0), damage(0.0),
	status_type((StatusType)-1)
{
}

void MonsterTolerance::print(QTextStream &stream, QString indent) const {
	XmlObject::print(stream, indent);
	stream << indent << "[" <<
		initial << ", " <<
		plus << ", " <<
		max << ", " <<
		regenValue << ", " <<
		regenTick << ", " <<
		duration << ", " <<
		damage << "]" << ENDL;
}

bool MonsterTolerance::readXmlElement(QXmlStreamReader *xml) {
	if (XmlObject::readXmlElement(xml)) return true;
	QStringRef tag_name = xml->name();
	if (tag_name == "initial") {
		initial = xml->readElementText().toDouble();
	} else if (tag_name == "plus") {
		plus = xml->readElementText().toDouble();
	} else if (tag_name == "max") {
		max = xml->readElementText().toDouble();
	} else if (tag_name == "regen_value") {
		regenValue = xml->readElementText().toDouble();
	} else if (tag_name == "regen_tick") {
		regenTick = xml->readElementText().toDouble();
	} else if (tag_name == "duration") {
		duration = xml->readElementText().toDouble();
	} else if (tag_name == "damage") {
		damage = xml->readElementText().toDouble();
	} else if (tag_name == "type") {
		QString t = xml->readElementText();
		for (int i = 0; i < STATUS_COUNT; ++i) {
			if (t == toString((StatusType)i)) {
				status_type = (StatusType)i;
			}
		}
	} else {
		return false;
	}
	return true;
}

Monster::Monster() : hitPoints(0.0) {
	std::fill_n(tolerances, (size_t)STATUS_COUNT, (MonsterTolerance *)NULL);
}

Monster::~Monster() {
	for (int i = 0; i < STATUS_COUNT; ++i) {
		delete tolerances[i];
	}
	foreach(MonsterPart *part, parts) delete part;
}

void Monster::print(QTextStream &stream, QString indent) const {
	NamedObject::print(stream, indent);
	stream << indent << "- hit points: " << hitPoints << ENDL;
	foreach(const MonsterPart *part, parts) {
		stream << indent << "- part" << ENDL;
		part->print(stream, indent + "\t");
	}
	for (int i = 0; i < STATUS_COUNT; ++i) {
		if (tolerances[i]) {
			stream << indent << "- tolerance: " << toString((StatusType)i) << ENDL;
			tolerances[i]->print(stream, indent + "\t");
		}
	}
}

bool Monster::readXmlElement(QXmlStreamReader *xml) {
	if (NamedObject::readXmlElement(xml)) return true;
	QStringRef tag_name = xml->name();
	if (tag_name == "hit_points") {
		hitPoints = xml->readElementText().toDouble();
	} else if (tag_name == "tolerance") {
		MonsterTolerance *tol = new MonsterTolerance;
		tol->readXml(xml);
		if (tol->status_type >= (StatusType)0 &&
		    tol->status_type < STATUS_COUNT) {
			tolerances[tol->status_type] = tol;
		} else {
			delete tol;
		}
	} else if (tag_name == "part") {
		MonsterPart *part = new MonsterPart;
		part->readXml(xml);
		parts.append(part);
	} else {
		return false;
	}
	return true;
}
