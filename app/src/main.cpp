#include "MhDpsApplication.h"

#include <QFile>
#include <QXmlStreamReader>
#include <QTextStream>

#include "Weapon.h"
#include "Monster.h"
#include "Dps.h"
#include "Profile.h"
#include "FoldedBuffs.h"
#include "Damage.h"
#include "Build.h"
#include "Item.h"
#include "BuffGroup.h"
#include "BuffWithCondition.h"
#include "DamageData.h"
#include "Target.h"
#include "Constants.h"

#include "MainData.h"

#include <stdio.h>
#include <QStringList>

static void gen_debug(QTextStream &stream, const MainData &data) {
	FILE *f = fopen("debug.csv", "w");
	if (!f) return;
	FILE *f2 = fopen("debug2.csv", "w");
	if (!f2) { fclose(f); return; }
	foreach(Weapon *weapon, data.weapons) {
		if (!weapon->final) continue;

		double element_crit_adjustment = 1.0;
		double status_crit_adjustment = 1.0;

		QHash<QString, double>::const_iterator cit;
		cit = Constants::instance()->elementCritAdjustment.find(weapon->type);
		if (cit != Constants::instance()->elementCritAdjustment.end()) {
			element_crit_adjustment = *cit;
		}
		cit = Constants::instance()->statusCritAdjustment.find(weapon->type);
		if (cit != Constants::instance()->statusCritAdjustment.end()) {
			status_crit_adjustment = *cit;
		}

		QVector<Item *> useful_items;
		foreach(Item *item, data.items) {
			bool useful = false;
			foreach(const Item::BuffRef &buff_ref, item->buffRefs) {
				if (!buff_ref.buffGroup) continue;
				foreach(BuffGroupLevel *bl, buff_ref.buffGroup->levels) {
					if (!bl) continue;
					foreach(BuffWithCondition *bc, bl->buffs) {
						if (bc->isUseful(*weapon)) {
							useful = true;
							break;
						}
					}
					if (useful) break;
				}
				if (useful) break;
			}
			if (useful) useful_items.append(item);
		}
		foreach(Profile *profile, data.profiles) {
			if (weapon->type != profile->type) continue;
			Build *build = new Build();
			build->addWeapon(weapon);
			build->addItem(data.itemHash["powercharm"]);
			build->addItem(data.itemHash["powertalon"]);
			//build->decorationSlots << 3;

			QVector<Build *> builds;
			builds << build;
			build->fillSlots(&builds, useful_items);
			foreach(Build *b, builds) {
				QVector<const BuffWithCondition *> bwc;
				b->getBuffWithConditions(&bwc);
				Damage dmg;
				foreach(Pattern *pattern, profile->patterns) {
					dmg.addPattern(bwc, *weapon, *pattern,
					               element_crit_adjustment,
					               status_crit_adjustment);
				}
				QStringList item_names;
				foreach(const Item *item, b->usedItems) {
					item_names.append(item->getName(NamedObject::LANG_EN));
				}
				item_names.sort();
				for (int i = 0; i < MODE_COUNT; ++i) {
					const char *mname = "";
					switch ((MonsterMode)i) {
					case MODE_NORMAL_NORMAL: mname = "not_enraged/normal_spot"; break;
					case MODE_ENRAGED_NORMAL: mname = "enraged/normal_spot"; break;
					case MODE_NORMAL_WEAK_SPOT: mname = "not_enraged/weak_spot"; break;
					case MODE_ENRAGED_WEAK_SPOT: mname = "enraged/weak_spot"; break;
					default: break;
					}
					const DamageData &dd = *dmg.data[i];
					double sharp_sum = 0.0;
					foreach(const SharpnessMultiplierData &smd, dd.bounceSharpness) {
						sharp_sum += smd.rate;
					}
					fprintf(f, "%s,%s,%s,%s,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g\n",
					       profile->getName(NamedObject::LANG_EN).toUtf8().data(),
					       weapon->getName(NamedObject::LANG_EN).toUtf8().data(),
					       item_names.join(" / ").toUtf8().data(),
					       mname,
					       dd.cut, dd.impact, dd.piercing, dd.fixed,
					       dd.elements[ELEMENT_FIRE],
					       dd.elements[ELEMENT_THUNDER],
					       dd.elements[ELEMENT_ICE],
					       dd.elements[ELEMENT_DRAGON],
					       dd.elements[ELEMENT_WATER],
					       dd.statuses[STATUS_POISON],
					       dd.statuses[STATUS_PARALYSIS],
					       dd.statuses[STATUS_SLEEP],
					       dd.statuses[STATUS_STUN],
					       dd.statuses[STATUS_BLAST],
					       dd.statuses[STATUS_EXHAUST],
					       dd.statuses[STATUS_MOUNT],
					       dd.mindsEyeRate / sharp_sum,
					       dd.bounceSharpness[0].rate / sharp_sum,
					       dd.bounceSharpness[0].multiplier);
				}
				foreach(Target *target, data.targets) {
					Dps target_dps(*target, dmg);
					fprintf(f2, "%s,%s,%s,%s,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g\n",
					       profile->getName(NamedObject::LANG_EN).toUtf8().data(),
					       weapon->getName(NamedObject::LANG_EN).toUtf8().data(),
					       item_names.join(" / ").toUtf8().data(),
					       target->getName(NamedObject::LANG_EN).toUtf8().data(),
					       target_dps.raw,
					       target_dps.total_elements,
					       target_dps.total_statuses,
					       target_dps.fixed,
					       target_dps.killFrequency,
					       target_dps.statusProcRate[STATUS_POISON],
					       target_dps.statusProcRate[STATUS_PARALYSIS],
					       target_dps.statusProcRate[STATUS_SLEEP],
					       target_dps.statusProcRate[STATUS_STUN],
					       target_dps.statusProcRate[STATUS_BLAST],
					       target_dps.statusProcRate[STATUS_EXHAUST],
					       target_dps.statusProcRate[STATUS_MOUNT],
					       target_dps.stunRate);
				}
			}
		}
	}
	fclose(f);
	fclose(f2);
}

static void do_stuff(QTextStream &stream, const MainData &data) {
	foreach(Weapon *weapon, data.weapons) {
		double element_crit_adjustment = 1.0;
		double status_crit_adjustment = 1.0;

		QHash<QString, double>::const_iterator cit;
		cit = Constants::instance()->elementCritAdjustment.find(weapon->type);
		if (cit != Constants::instance()->elementCritAdjustment.end()) {
			element_crit_adjustment = *cit;
		}
		cit = Constants::instance()->statusCritAdjustment.find(weapon->type);
		if (cit != Constants::instance()->statusCritAdjustment.end()) {
			status_crit_adjustment = *cit;
		}

		QVector<Item *> useful_items;
		foreach(Item *item, data.items) {
			bool useful = false;
			foreach(const Item::BuffRef &buff_ref, item->buffRefs) {
				if (!buff_ref.buffGroup) continue;
				foreach(BuffGroupLevel *bl, buff_ref.buffGroup->levels) {
					if (!bl) continue;
					foreach(BuffWithCondition *bc, bl->buffs) {
						if (bc->isUseful(*weapon)) {
							useful = true;
							break;
						}
					}
					if (useful) break;
				}
				if (useful) break;
			}
			if (useful) useful_items.append(item);
		}
		foreach(Profile *profile, data.profiles) {
			if (weapon->type != profile->type) continue;
			Build *build = new Build();
			build->addWeapon(weapon);
			build->addItem(data.itemHash["powercharm"]);
			build->addItem(data.itemHash["powertalon"]);
//			build->decorationSlots << 3;

			QVector<Build *> builds;
			builds << build;
			build->fillSlots(&builds, useful_items);
			foreach(Build *b, builds) {
				QVector<const BuffWithCondition *> bwc;
				b->getBuffWithConditions(&bwc);
				Damage dmg;
				foreach(Pattern *pattern, profile->patterns) {
					dmg.addPattern(bwc, *weapon, *pattern,
					               element_crit_adjustment,
					               status_crit_adjustment);
				}

				bool first = true;
				stream << weapon->getName(NamedObject::LANG_EN) << ": ";
				foreach(const Item *item, b->usedItems) {
					if (first) {
						first = false;
					} else {
						stream << ", ";
					}
					stream << item->getName(NamedObject::LANG_EN);
				}
				stream << endl;
				dmg.print(stream, "\t");

				foreach(Target *target, data.targets) {
					Dps target_dps(*target, dmg);
					stream <<  "\t- target: " << target->getName(NamedObject::LANG_EN) << endl;
					target_dps.print(stream, "\t\t");
				}

				delete b;
			}
		}
	}
}

int main(int argc, char **argv)
{
	MainData data;
	for (int i = 1; i < argc; ++i) {
		QFile f(argv[i]);
		if (f.open(QFile::ReadOnly)) {
			QXmlStreamReader xml(&f);
			while (!xml.atEnd()) {
				QXmlStreamReader::TokenType token_type = xml.readNext();
				if (token_type == QXmlStreamReader::StartElement) {
					if (xml.name() == "data") {
						data.readXml(&xml);
					}
				}
			}
		}
	}
	data.matchData();

	QTextStream stream(stdout);
	stream.setCodec("UTF-8");
//	data.print(stream);
	gen_debug(stream, data);
//	do_stuff(stream, data);
	return 0;
	// MhDpsApplication app(argc, argv);
	// return app.exec();
}
