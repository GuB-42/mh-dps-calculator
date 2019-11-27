#ifndef MainApplication_h_
#define MainApplication_h_

#include <QApplication>

struct MainData;
class MainWindow;

class MainApplication : public QApplication
{
	Q_OBJECT
public:
	MainApplication(int &argc, char *argv[]);
	~MainApplication();

	QTranslator *translator() const { return m_translator; }
	QTranslator *translatorQt() const { return m_translatorQt; }
private:
	MainWindow *mainWindow;
	MainData *mainData;
	QTranslator *m_translator;
	QTranslator *m_translatorQt;
};

#endif
