#ifndef MhDpsApplication_h_
#define MhDpsApplication_h_

#include <QApplication>

class MhDpsApplication : public QApplication
{
	Q_OBJECT
public:
	MhDpsApplication(int &argc, char *argv[]);
};

#endif
