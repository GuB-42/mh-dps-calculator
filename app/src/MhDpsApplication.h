#ifndef MhDpsApplication_h_
#define MhDpsApplication_h_

#include <QApplication>

struct MainData;
class MainWindow;

class MhDpsApplication : public QApplication
{
	Q_OBJECT
public:
	MhDpsApplication(int &argc, char *argv[]);
	~MhDpsApplication();
private:
	MainWindow *mainWindow;
	MainData *mainData;
};

#endif
