#include "MhDpsApplication.h"

#include <QFile>
#include <QXmlStreamReader>
#include <QTextStream>

#include "Weapon.h"
#include "Profile.h"
#include "FoldedBuffs.h"
#include "DamageData.h"

#include "MainData.h"

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
	foreach(Weapon *weapon, data.weapons) {
		foreach(Profile *profile, data.profiles) {
			if (weapon->type != profile->type) continue;
	//		if (weapon->getName(NamedObject::LANG_EN) != "Rathsblade I") continue;
			foreach(Pattern *pattern, profile->patterns) {
				DamageData dmg(*weapon, FoldedBuffsData(), *pattern, 1.0);

				stream << "[ " << weapon->getName(NamedObject::LANG_EN) << " / " <<
					profile->getName(NamedObject::LANG_EN) << "]" << endl;
				stream << "- weapon" << endl;
				weapon->print(stream, "\t");
				stream << "- pattern" << endl;
				pattern->print(stream, "\t");
				stream << "- damage" << endl;
				dmg.print(stream, "\t");
				stream << endl;
			}
		}
	}
	return 0;
	// MhDpsApplication app(argc, argv);
	// return app.exec();
}
