#include "Target.h"

#include <QTextStream>
#include <QXmlStreamReader>
#include "QtCompatibility.h"
#include <QHash>
#include "Monster.h"
#include "Constants.h"

TargetZone::TargetZone() :
	weight(0.0), expectedEnragedRatio(0.0), enragedRatio(0.0),
	part(NULL), hitData(NULL)
{
}

void TargetZone::print(QTextStream &stream, QString indent) const {
	stream << indent << "- weight: " << weight << endl;
	stream << indent << "- enraged ratio: " << enragedRatio << endl;
	stream << indent << "- expected enraged ratio: " << expectedEnragedRatio << endl;
	stream << indent << "- part: " << part->getAllNames() << endl;
	stream << indent << "- hit data" << endl;
	hitData->print(stream, indent + "\t");
}

TargetMonster::TargetMonster() :
	defenseMultiplier(0.0), statusDefenseMultiplier(0.0),
	statusHitMultiplier(0.0), monster(NULL)
{
}

TargetMonster::~TargetMonster() {
	foreach (TargetZone *z, targetZones) delete z;
}

void TargetMonster::print(QTextStream &stream, QString indent) const {
	stream << indent << "- defense multiplier: " << defenseMultiplier << endl;
	stream << indent << "- status defense multiplier: " << statusDefenseMultiplier << endl;
	stream << indent << "- status hit multiplier: " << statusHitMultiplier << endl;
	stream << indent << "- monster: " << monster->getAllNames() << endl;
	foreach(const TargetZone *targetZone, targetZones) {
		stream << indent << "- zone" << endl;
		targetZone->print(stream, indent + "\t");
	}
}

Target::SubTarget::SubTarget() :
	weight(1.0),
	hasDefenseMultiplier(false), defenseMultiplier(0.0),
	hasStatusDefenseMultiplier(false), statusDefenseMultiplier(0.0),
	hasStatusHitMultiplier(false), statusHitMultiplier(0.0),
	hasEnragedRatio(false), enragedRatio(0.0)
{
}

Target::SubTarget::~SubTarget() {
	foreach(SubTarget *subTarget, subTargets) delete subTarget;
}

void Target::SubTarget::print(QTextStream &stream, QString indent) const {
	stream << indent << "- weight: " << weight << endl;
	if (hasEnragedRatio) {
		stream << indent << "- enraged ratio: " << enragedRatio << endl;
	}
	if (hasDefenseMultiplier) {
		stream << indent << "- defense multiplier: " << defenseMultiplier << endl;
	}
	if (hasStatusDefenseMultiplier) {
		stream << indent << "- status defense multiplier: " << statusDefenseMultiplier << endl;
	}
	if (hasStatusHitMultiplier) {
		stream << indent << "- status hit multiplier: " << statusHitMultiplier << endl;
	}
	if (!monsterId.isNull()) {
		stream << indent << "- monster: " << monsterId << endl;
	}
	if (!partId.isNull()) {
		stream << indent << "- part: " << partId << endl;
	}
	if (!stateId.isNull()) {
		stream << indent << "- state: " << stateId << endl;
	}
	foreach(const SubTarget *subTarget, subTargets) {
		stream << indent << "- sub target" << endl;
		subTarget->print(stream, indent + "\t");
	}
}

void Target::SubTarget::readXml(QXmlStreamReader *xml) {
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			QStringRef tag_name = xml->name();
			if (tag_name == "weight") {
				weight = xml->readElementText().toDouble();
			} else if (tag_name == "enraged_ratio") {
				enragedRatio = xml->readElementText().toDouble();
				hasEnragedRatio = true;
			} else if (tag_name == "monster_defense_multiplier") {
				defenseMultiplier = xml->readElementText().toDouble();
				hasDefenseMultiplier = true;
			} else if (tag_name == "monster_status_defense_multiplier") {
				statusDefenseMultiplier = xml->readElementText().toDouble();
				hasStatusDefenseMultiplier = true;
			} else if (tag_name == "status_hit_multiplier") {
				statusHitMultiplier = xml->readElementText().toDouble();
				hasStatusHitMultiplier = true;
			} else if (tag_name == "monster_name") {
				monsterId = xml->readElementText();
			} else if (tag_name == "part_name") {
				partId = xml->readElementText();
			} else if (tag_name == "state") {
				stateId = xml->readElementText();
				if (stateId.isNull()) stateId = "";
			} else if (tag_name == "sub_target") {
				SubTarget *st = new SubTarget();
				st->readXml(xml);
				subTargets.append(st);
			} else {
				XML_SKIP_CURRENT_ELEMENT(*xml);
			}
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
}

Target::~Target() {
	foreach(TargetMonster *tm, targetMonsters) delete tm;
}

void Target::print(QTextStream &stream, QString indent) const {
	NamedObject::print(stream, indent);
	stream << indent << "- sub target" << endl;
	rootSubTarget.print(stream, indent + "\t");
	foreach(const TargetMonster *targetMonster, targetMonsters) {
		stream << indent << "- monster" << endl;
		targetMonster->print(stream, indent + "\t");
	}
}

void Target::readXml(QXmlStreamReader *xml) {
	if (xml->attributes().hasAttribute("id")) {
		id = xml->attributes().value("id").toString();
	}
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			QStringRef tag_name = xml->name();
			if (readXmlName(xml)) {
				; // name
			} else if (tag_name == "enraged_ratio") {
				rootSubTarget.enragedRatio =
					xml->readElementText().toDouble();
				rootSubTarget.hasEnragedRatio = true;
			} else if (tag_name == "monster_defense_multiplier") {
				rootSubTarget.defenseMultiplier =
					xml->readElementText().toDouble();
				rootSubTarget.hasDefenseMultiplier = true;
			} else if (tag_name == "monster_status_defense_multiplier") {
				rootSubTarget.statusDefenseMultiplier =
					xml->readElementText().toDouble();
				rootSubTarget.hasStatusDefenseMultiplier = true;
			} else if (tag_name == "status_hit_multiplier") {
				rootSubTarget.statusHitMultiplier =
					xml->readElementText().toDouble();
				rootSubTarget.hasStatusHitMultiplier = true;
			} else if (tag_name == "monster_name") {
				rootSubTarget.monsterId = xml->readElementText();
			} else if (tag_name == "part_name") {
				rootSubTarget.partId = xml->readElementText();
			} else if (tag_name == "state") {
				rootSubTarget.stateId = xml->readElementText();
			} else if (tag_name == "sub_target") {
				SubTarget *st = new SubTarget();
				st->readXml(xml);
				rootSubTarget.subTargets.append(st);
			} else {
				XML_SKIP_CURRENT_ELEMENT(*xml);
			}
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
}

struct SubTargetLeaf {
	double weight;
	double defenseMultiplier;
	double statusDefenseMultiplier;
	double statusHitMultiplier;
	double enragedRatio;
	QString monsterId;
	QString partId;
	QString stateId;
};

static void make_flat_subs(QVector<SubTargetLeaf> *subs,
                           const Target::SubTarget *st,
                           SubTargetLeaf data) {
	data.weight *= st->weight;
	if (st->hasDefenseMultiplier) {
		data.defenseMultiplier = st->defenseMultiplier;
	}
	if (st->hasStatusDefenseMultiplier) {
		data.statusDefenseMultiplier = st->statusDefenseMultiplier;
	}
	if (st->hasStatusHitMultiplier) {
		data.statusHitMultiplier = st->statusHitMultiplier;
	}
	if (st->hasEnragedRatio) {
		data.enragedRatio = st->enragedRatio;
	}
	if (!st->monsterId.isNull()) data.monsterId = st->monsterId;
	if (!st->partId.isNull()) data.partId = st->partId;
	if (!st->stateId.isNull()) data.stateId = st->stateId;
	if (st->subTargets.isEmpty()) {
		subs->append(data);
	} else {
		foreach(const Target::SubTarget *sst, st->subTargets) {
			make_flat_subs(subs, sst, data);
		}
	}
}

static TargetZone *make_target_zone(const SubTargetLeaf &subt,
                                    const MonsterPart *part,
                                    const MonsterHitData *hitData) {

	TargetZone *ret = new TargetZone();
	ret->weight = subt.weight;
	ret->expectedEnragedRatio = subt.enragedRatio;
	double sr = subt.enragedRatio * hitData->enragedState;
	double isr = (1 - subt.enragedRatio) * (1 - hitData->enragedState);
	ret->enragedRatio = sr / (sr + isr);
	ret->part = part;
	ret->hitData = hitData;
	return ret;
}

static void merge_target_zones(QVector<TargetZone *> *oval,
                               QVector<TargetZone *> *nval,
                               const QVector<MonsterPart *> &monsterParts) {
	QVector<TargetZone *>::iterator oz_it = oval->begin();
	QVector<TargetZone *>::iterator nz_it = nval->begin();
	foreach(MonsterPart *part, monsterParts) {
		if (nz_it == nval->end()) goto end_merge_zone;
		if (oz_it == oval->end()) goto end_merge_zone;
		while ((*oz_it)->part == part) {
			if ((*nz_it)->part == part) {
				foreach(MonsterHitData *hitData, part->hitData) {
					if ((*oz_it)->hitData == hitData) {
						if ((*nz_it)->hitData == hitData) {
							double new_weight =
								(*oz_it)->weight + (*nz_it)->weight;
							if (new_weight != 0.0) {
								(*oz_it)->enragedRatio =
									((*oz_it)->enragedRatio * (*oz_it)->weight +
									 (*nz_it)->enragedRatio * (*nz_it)->weight) /
									new_weight;
								(*oz_it)->expectedEnragedRatio =
									((*oz_it)->expectedEnragedRatio * (*oz_it)->weight +
									 (*nz_it)->expectedEnragedRatio * (*nz_it)->weight) /
									new_weight;
							}
							(*oz_it)->weight = new_weight;
							delete *nz_it++;
							if (nz_it == nval->end()) goto end_merge_zone;
						}
						++oz_it;
						if (oz_it == oval->end()) goto end_merge_zone;
					} else if ((*nz_it)->hitData == hitData) {
						oz_it = oval->insert(oz_it, *nz_it++);
						++oz_it;
						if (nz_it == nval->end()) goto end_merge_zone;
					}
				}
			} else {
				++oz_it;
				if (oz_it == oval->end()) goto end_merge_zone;
			}
		}
		while ((*nz_it)->part == part) {
			oz_it = oval->insert(oz_it, *nz_it++);
			++oz_it;
			if (nz_it == nval->end()) goto end_merge_zone;
		}
	}
end_merge_zone:
	while (nz_it != nval->end()) {
		oval->append(*nz_it++);
	}
}

static void merge_target_monsters(QVector<TargetMonster *> *oval,
                                  QVector<TargetMonster *> *nval,
                                  const QVector<Monster *> &monsters) {
	QVector<TargetMonster *>::iterator om_it = oval->begin();
	QVector<TargetMonster *>::iterator nm_it = nval->begin();
	foreach(Monster *monster, monsters) {
		if (nm_it == nval->end()) goto end_merge_monster;
		if (om_it == oval->end()) goto end_merge_monster;
		while ((*om_it)->monster == monster) {
			if ((*nm_it)->monster == monster) {
				if ((*om_it)->defenseMultiplier == (*nm_it)->defenseMultiplier) {
					merge_target_zones(&(*om_it)->targetZones,
					                   &(*nm_it)->targetZones,
					                   monster->parts);
					(*nm_it)->targetZones.clear();
					delete *nm_it++;
				} else if ((*om_it)->defenseMultiplier > (*nm_it)->defenseMultiplier) {
					om_it = oval->insert(om_it, *nm_it++);
					++om_it;
				}
				if (nm_it == nval->end()) goto end_merge_monster;
			}
			++om_it;
			if (om_it == oval->end()) goto end_merge_monster;
		}
		if ((*nm_it)->monster == monster) {
			om_it = oval->insert(om_it, *nm_it++);
			++om_it;
		}
	}
end_merge_monster:
	while (nm_it != nval->end()) {
		oval->append(*nm_it++);
	}
}

static void fix_enraged_weights(QVector<TargetZone *> targetZones) {
	QVector<TargetZone *>::const_iterator it_start = targetZones.begin();
	QVector<TargetZone *>::const_iterator it_end = it_start;
	while (it_start != targetZones.end()) {
		while (it_end != targetZones.end() &&
		       (*it_end)->part == (*it_start)->part) ++it_end;
		// weird maths that kinda work...
		for (int i = 0; i < 10; ++i) {
			double total_weight = 0.0;
			double er = 0.0;
			double k = 0.0;
			double ik = 0.0;
			int count = 0;
			for (QVector<TargetZone *>::const_iterator it = it_start;
			     it != it_end; ++it) {
				total_weight += (*it)->weight;
				er += (*it)->expectedEnragedRatio * (*it)->weight;
				k += (*it)->enragedRatio * (*it)->weight;
				ik += (1.0 - (*it)->enragedRatio) * (*it)->weight;
				++count;
			}
			if (count > 1 && k != 0.0 && ik != 0.0 && total_weight != 0.0 &&
			    (er - k) * (er - k) > 1e-6 * total_weight * total_weight) {
				er /= total_weight;
				k /= total_weight;
				ik /= total_weight;
				for (QVector<TargetZone *>::const_iterator it = it_start;
				     it != it_end; ++it) {
					double w = (*it)->enragedRatio;
					double factor = w * (er / k) + (1 - w) * ((1 - er) / ik);
					(*it)->weight *= factor;
				}
			} else {
				break;
			}
		}
		it_start = it_end;
	}
}

void Target::matchMonsters(const QVector<Monster *> &monsters) {
	QVector<SubTargetLeaf> flat_subs;
	SubTargetLeaf data = {
		1.0,
		Constants::instance()->monsterDefenseMultiplier,
		Constants::instance()->monsterDefenseMultiplier,
		1.0,
		Constants::instance()->enragedRatio,
		QString(),
		QString(),
		QString()
	};
	make_flat_subs(&flat_subs, &rootSubTarget, data);

	double total_weight = 0.0;
	foreach(const SubTargetLeaf &subt, flat_subs) {
		total_weight += subt.weight;
	}
	for (int i = 0; i < flat_subs.count(); ++i) {
		flat_subs[i].weight /= total_weight;
	}

	foreach(const SubTargetLeaf &subt, flat_subs) {
		QVector<TargetMonster *> tmonsters;

		double total_divider = 0.0;
		foreach(Monster *monster, monsters) {
			if (!subt.monsterId.isNull() && subt.monsterId != // FIXME : use ids
			    monster->getName(LANG_EN)) continue;
			TargetMonster *target_monster = NULL;
			double monster_divider = 0.0;
			foreach(MonsterPart *part, monster->parts) {
				if (!subt.partId.isNull() && subt.partId != // FIXME : use ids
				    part->getName(LANG_EN)) continue;
				double part_divider = 0.0;
				int part_idx =
					target_monster ? target_monster->targetZones.count() : 0;
				bool has_states = false;
				foreach(MonsterHitData *hitData, part->hitData) {
					if (!hitData->state.isNull()) {
						has_states = true;
						break;
					}
				}
				foreach(MonsterHitData *hitData, part->hitData) {
					if (!has_states || subt.stateId.isNull() ||
					    subt.stateId == hitData->state) {
						if (!target_monster) {
							target_monster = new TargetMonster();
							target_monster->defenseMultiplier =
								subt.defenseMultiplier;
							target_monster->statusDefenseMultiplier =
								subt.statusDefenseMultiplier;
							target_monster->statusHitMultiplier =
								subt.statusHitMultiplier;
							target_monster->monster = monster;
							tmonsters.append(target_monster);
						}
						target_monster->targetZones.
							append(make_target_zone(subt, part, hitData));
						part_divider += 1.0;
					}
				}
				if (part_divider > 0.0) {
					for (int i = part_idx;
					     i < target_monster->targetZones.count(); ++i) {
						target_monster->targetZones[i]->weight /= part_divider;
					}
					monster_divider += 1.0;
				}
			}
			if (monster_divider > 0.0) {
				for (int i = 0; i < target_monster->targetZones.count(); ++i) {
					target_monster->targetZones[i]->weight /= monster_divider;
				}
				total_divider += 1.0;
			}
		}
		if (total_divider > 1.0) {
			for (int i = 0; i < tmonsters.count(); ++i) {
				for (int j = 0; j < tmonsters[i]->targetZones.count(); ++j) {
					tmonsters[i]->targetZones[j]->weight /= total_divider;
				}
			}
		}

		merge_target_monsters(&targetMonsters, &tmonsters, monsters);
	}
	foreach(TargetMonster *tmonster, targetMonsters) {
		fix_enraged_weights(tmonster->targetZones);
	}
}
