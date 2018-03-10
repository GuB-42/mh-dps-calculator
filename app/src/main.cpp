#include "MhDpsApplication.h"

#include <QFile>
#include <QXmlStreamReader>
#include <QTextStream>

#include "Weapon.h"
#include "Profile.h"
#include "FoldedBuffs.h"
#include "Damage.h"

#include "MainData.h"

static void do_stuff(QTextStream &stream, const MainData &data) {
	foreach(Weapon *weapon, data.weapons) {
		foreach(Profile *profile, data.profiles) {
			if (weapon->type != profile->type) continue;
	//		if (weapon->getName(NamedObject::LANG_EN) != "Rathsblade I") continue;
			Damage dmg;
			foreach(Pattern *pattern, profile->patterns) {
				dmg.addPattern(QVector<const BuffWithCondition *>(),
				               *weapon, *pattern, 1.0);
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
	QTextStream stream(stdout);
	stream.setCodec("UTF-8");
//	data.print(stream);
	do_stuff(stream, data);
	return 0;
	// MhDpsApplication app(argc, argv);
	// return app.exec();
}
