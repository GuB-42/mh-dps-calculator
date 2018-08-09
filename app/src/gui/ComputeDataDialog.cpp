#include "ComputeDataDialog.h"
#include "ui_ComputeDataDialog.h"

#include "ComputeDataModel.h"
#include "../Target.h"
#include "../Profile.h"

ComputeDataDialog::ComputeDataDialog(const Profile *p, const Target *t,
                                     QWidget *parent) :
	QDialog(parent), ui(new Ui::ComputeDataDialog),
	model(new ComputeDataModel(p, t, this))
{
	ui->setupUi(this);
	ui->tableView->setModel(model);
	ui->tableView->resizeColumnsToContents();
}

ComputeDataDialog::~ComputeDataDialog() {
	delete ui;
}

void ComputeDataDialog::changeEvent(QEvent *event) {
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
