#ifndef DataPrinter_h_
#define DataPrinter_h_

#include <QCoreApplication>
#include <QString>
#include "../enums.h"

struct BuildWithDps;

class DataPrinter {
	Q_DECLARE_TR_FUNCTIONS(DataPrinter);
public:
	static QString singleDataToHtml(BuildWithDps *bwd, Language lang);
	static QString dataToHtml(const QVector<BuildWithDps *> &bwds,
	                          Language lang);
};

#endif
