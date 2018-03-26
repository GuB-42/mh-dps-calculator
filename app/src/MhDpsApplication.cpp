#include "MhDpsApplication.h"

#include <QtCore/QtGlobal>
#include <QFile>
#include <QDir>
#include <QXmlStreamReader>

#include "gui/MainWindow.h"
#include "MainData.h"

MhDpsApplication::MhDpsApplication(int &argc, char *argv[]) :
	QApplication(argc, argv)
{
	mainData = new MainData;
	{
		QDir data_dir("data");
		if (data_dir.exists()) {
			foreach (QString filename,
			         data_dir.entryList(QStringList() << "*.xml",
			                            QDir::Files)) {
				QFile f(data_dir.filePath(filename));
				if (f.open(QFile::ReadOnly)) {
					QXmlStreamReader xml(&f);
					while (!xml.atEnd()) {
						QXmlStreamReader::TokenType token_type = xml.readNext();
						if (token_type == QXmlStreamReader::StartElement) {
							if (xml.name() == "data") {
								mainData->readXml(&xml);
							}
						}
					}
				}
			}
		}
	}
	for (int i = 1; i < argc; ++i) {
		QFile f(argv[i]);
		if (f.open(QFile::ReadOnly)) {
			QXmlStreamReader xml(&f);
			while (!xml.atEnd()) {
				QXmlStreamReader::TokenType token_type = xml.readNext();
				if (token_type == QXmlStreamReader::StartElement) {
					if (xml.name() == "data") {
						mainData->readXml(&xml);
					}
				}
			}
		}
	}
	mainData->matchData();

	mainWindow = new MainWindow;
	mainWindow->setMainData(mainData);
	mainWindow->show();
}

MhDpsApplication::~MhDpsApplication()
{
	mainWindow->setMainData(NULL);
	delete mainWindow;
	delete mainData;
}
