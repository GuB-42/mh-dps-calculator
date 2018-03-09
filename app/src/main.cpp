#include "MhDpsApplication.h"

#include <QFile>
#include <QXmlStreamReader>
#include <QTextStream>

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
	data.print(stream);
	return 0;
	// MhDpsApplication app(argc, argv);
	// return app.exec();
}
