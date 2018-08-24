#include "DetailsDialog.h"
#include "ui_DetailsDialog.h"

#include "../BuildWithDps.h"
#include "DataPrinter.h"

DetailsDialog::DetailsDialog(const Profile *profile,
                             const Target *target,
                             QVector<BuildWithDps *> rd,
                             Language lang,
                             QWidget *parent) :
	QDialog(parent), ui(new Ui::DetailsDialog),
	dataLanguage(lang)
{
	ui->setupUi(this);
	resultData.reserve(rd.count());
	foreach(BuildWithDps *bwd, rd) {
		BuildWithDps *n = new BuildWithDps(*bwd);
		n->compute(*profile, *target, true);
		resultData.append(n);
	}
	updateContent();
}

DetailsDialog::~DetailsDialog() {
	delete ui;
	foreach(BuildWithDps *bwd, resultData) delete bwd;
}

void DetailsDialog::setDataLanguage(Language lang) {
	dataLanguage = lang;
	updateContent();
}

void DetailsDialog::changeEvent(QEvent *event) {
	if (event) {
		switch (event->type()) {
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;
		default:
			break;
		}
	}
	QDialog::changeEvent(event);
}

void DetailsDialog::updateContent() {
	ui->textBrowser->
		setHtml(DataPrinter::dataToHtml(resultData, dataLanguage));
}
