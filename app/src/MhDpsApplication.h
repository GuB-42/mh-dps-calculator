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
private:
	MainWindow *mainWindow;
	MainData *mainData;
};

#endif
