#include "MhDpsApplication.h"

#include <QFile>
#include <QXmlStreamReader>
#include <QTextStream>

#include "Weapon.h"
#include "Profile.h"
#include "FoldedBuffs.h"
#include "Damage.h"
#include "Build.h"
#include "Item.h"
#include "BuffGroup.h"

#include "MainData.h"

static void do_stuff(QTextStream &stream, const MainData &data) {
	foreach(Weapon *weapon, data.weapons) {
		QVector<Item *> useful_items;
		foreach(Item *item, data.items) {
			bool useful = false;
			foreach(const Item::BuffRef &buff_ref, item->buffRefs) {
				if (!buff_ref.buffGroup) continue;
				foreach(BuffGroupLevel *bl, buff_ref.buffGroup->levels) {
					if (!bl) continue;
					foreach(BuffWithCondition *bc, bl->buffs) {
						useful = true;
						break;
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
			build->decorationSlots << 3;

			QVector<Build *> builds;
			builds << build;
			build->fillSlots(&builds, useful_items);
			foreach(Build *b, builds) {
				QVector<const BuffWithCondition *> bwc;
				b->getBuffWithConditions(&bwc);
				Damage dmg;
				foreach(Pattern *pattern, profile->patterns) { /*
					bool raw_weapon = true;
					for (int i = 0; i < ELEMENT_COUNT; ++i) {
						if (weapon->elements[i] > 0.0) raw_weapon = false;
					}
					for (int i = 0; i < STATUS_COUNT; ++i) {
						if (weapon->statuses[i] > 0.0) raw_weapon = false;
					}
					FoldedBuffs folded_buffs(bwc,
					                         *pattern->conditionRatios,
						                     raw_weapon, weapon->awakened,
						                     weapon->affinity);
					folded_buffs.print(stream);
*/
					dmg.addPattern(bwc, *weapon, *pattern, 1.0);
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

//				dmg.print(stream, "\t");

				delete b;
			}
		}
	}
/*
			stream << "[ " << weapon->getName(NamedObject::LANG_EN) << " / " <<
				profile->getName(NamedObject::LANG_EN) << "]" << endl;
			stream << "- weapon" << endl;
			weapon->print(stream, "\t");
			stream << "- pattern" << endl;
			profile->print(stream, "\t");
			stream << "- damage" << endl;
			dmg.print(stream, "\t");
			stream << endl;*/
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
	do_stuff(stream, data);
	return 0;
	// MhDpsApplication app(argc, argv);
	// return app.exec();
}
