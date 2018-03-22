#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QSortFilterProxyModel>
#include <QClipboard>

#include "ResultTableModel.h"
#include "../MainData.h"
#include "../Target.h"
#include "../Profile.h"
#include "../Weapon.h"
#include "../Build.h"
#include "../BuildWithDps.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	mainData(NULL),
	tableModel(new ResultTableModel(this)),
	dataLanguage(NamedObject::LANG_EN)
{
	ui->setupUi(this);

	for (int i = 0; i < NamedObject::LANG_COUNT; ++i) {
		ui->languageCb->addItem(toString((NamedObject::Language)i));
	}
	connect(ui->languageCb, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(changeLanguage(int)));

	tableModel->setDataLanguage(dataLanguage);
	ui->tableView->setModel(tableModel);
	ui->tableView->addAction(ui->action_Copy);

	connect(ui->calculateDps, SIGNAL(clicked()), this, SLOT(calculate()));
	connect(ui->action_Quit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui->action_Copy, SIGNAL(triggered()), this, SLOT(copy()));

	connect(QApplication::instance(), SIGNAL(focusChanged(QWidget*, QWidget*)),
	        this, SLOT(updateCopyAction()));
	connect(ui->tableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
	        this, SLOT(updateCopyAction()));
	updateCopyAction();
}

MainWindow::~MainWindow() {
	delete ui;
}

void MainWindow::clearAll() {
	ui->profileCb->clear();
	ui->targetCb->clear();
}

void MainWindow::setMainData(const MainData *md) {
	clearAll();
	mainData = md;
	if (!mainData) return;

	for (int idx = 0; idx < mainData->profiles.count(); ++idx) {
		ui->profileCb->addItem(mainData->profiles[idx]->getName(dataLanguage));
	}
	for (int idx = 0; idx < mainData->targets.count(); ++idx) {
		ui->targetCb->addItem(mainData->targets[idx]->getName(dataLanguage));
	}
}


const Profile *MainWindow::getProfile() const {
	int idx = ui->profileCb->currentIndex();
	if (mainData && idx >= 0 && idx < mainData->profiles.count()) {
		return mainData->profiles[idx];
	} else {
		return NULL;
	}
}

const Target *MainWindow::getTarget() const {
	int idx = ui->targetCb->currentIndex();
	if (mainData && idx >= 0 && idx < mainData->targets.count()) {
		return mainData->targets[idx];
	} else {
		return NULL;
	}
}

QVector<int> MainWindow::getDecorationSlots() const {
	QVector<int> ret;
	for (int i = 0; i < ui->level3Slots->value(); ++i) ret.append(3);
	for (int i = 0; i < ui->level2Slots->value(); ++i) ret.append(2);
	for (int i = 0; i < ui->level1Slots->value(); ++i) ret.append(1);
	return ret;
}

void MainWindow::setDataLanguage(NamedObject::Language lang) {
	dataLanguage = lang;
	tableModel->setDataLanguage(dataLanguage);
	for (int idx = 0; idx < ui->profileCb->count(); ++idx) {
		if (mainData && idx < mainData->profiles.count()) {
			ui->profileCb->setItemText(idx, mainData->profiles[idx]->
			                           getName(dataLanguage));
		}
	}
	for (int idx = 0; idx < ui->targetCb->count(); ++idx) {
		if (mainData && idx < mainData->targets.count()) {
			ui->targetCb->setItemText(idx, mainData->targets[idx]->
			                          getName(dataLanguage));
		}
	}
}

void MainWindow::updateCopyAction() {
	if (ui->tableView->selectionModel()->hasSelection()) {
		ui->action_Copy->setEnabled(true);
	} else {
		ui->action_Copy->setEnabled(false);
	}
}

void MainWindow::changeLanguage(int lang_idx) {
	if (lang_idx >= 0 && lang_idx < NamedObject::LANG_COUNT) {
		setDataLanguage((NamedObject::Language)lang_idx);
	}
}

void MainWindow::calculate() {
	const Profile *profile = getProfile();
	const Target *target = getTarget();

	if (!profile || !target) return;

	QVector<BuildWithDps *> result;
	foreach(const Weapon *weapon, mainData->weapons) {
		if (weapon->type != profile->type) continue;
		if (!weapon->final && ui->finalOnly->isChecked()) continue;

		Build *build = new Build;
		build->addWeapon(weapon);
		if (ui->ignoreWeaponSlots->isChecked()) {
			build->decorationSlots.clear();
		}
		build->buffLevels[mainData->buffGroupHash["powercharm"]] = 1;
		build->buffLevels[mainData->buffGroupHash["powertalon"]] = 1;
		build->decorationSlots << getDecorationSlots();

		QVector<Build *> builds;
		builds << build;
		build->fillSlots(&builds, build->listUsefulItems(mainData->items));
		foreach(Build *b, builds) {
			result.append(new BuildWithDps(b, *profile, *target));
		}
	}
	tableModel->setResultData(result);
}

void MainWindow::copy() {
	if (ui->tableView->hasFocus() && ui->tableView->selectionModel()) {
		QModelIndexList indexes;
		foreach(const QModelIndex &index,
		        ui->tableView->selectionModel()->selectedIndexes()) {
			if (index.model() == ui->tableView->model()) {
				indexes.append(index);
			}
		}
		QApplication::clipboard()->
			setMimeData(ui->tableView->model()->mimeData(indexes));
	}
}
