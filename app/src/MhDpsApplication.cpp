#include "MhDpsApplication.h"

#include <QtCore/QtGlobal>

#include <stdio.h>

MhDpsApplication::MhDpsApplication(int &argc, char *argv[]) :
	QApplication(argc, argv)
{
	printf("hello world\n");
}
