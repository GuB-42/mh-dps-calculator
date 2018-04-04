#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QClipboard>

#include "ResultTableModel.h"
#include "BuffListModel.h"
#include "../MainData.h"
#include "../Target.h"
#include "../Profile.h"
#include "../Weapon.h"
#include "../Build.h"
#include "../Item.h"
#include "../BuffGroup.h"
#include "../BuildWithDps.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	mainData(NULL),
	tableModel(new ResultTableModel(this)),
	buffListModel(new BuffListModel(this)),
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

	buffListModel->setDataLanguage(dataLanguage);
	ui->buffListView->setModel(buffListModel);

	connect(ui->buffListView->selectionModel(),
	        SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
	        this, SLOT(updateBuffGroupFromListSelection()));
	connect(ui->buffCb, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(buffGroupChanged(int)));

	connect(ui->calculateDps, SIGNAL(clicked()), this, SLOT(calculate()));
	connect(ui->action_Quit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui->action_Copy, SIGNAL(triggered()), this, SLOT(copy()));

	connect(ui->addBuff, SIGNAL(clicked()), this, SLOT(addBuff()));
	connect(ui->removeBuff, SIGNAL(clicked()), this, SLOT(removeBuff()));

	connect(QApplication::instance(), SIGNAL(focusChanged(QWidget*, QWidget*)),
	        this, SLOT(updateCopyAction()));
	connect(ui->tableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
	        this, SLOT(updateCopyAction()));
	updateCopyAction();

	ui->splitter->setSizes(QList<int>() << 1 << height());
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
	for (int idx = 0; idx < mainData->buffGroups.count(); ++idx) {
		ui->buffCb->addItem(mainData->buffGroups[idx]->getName(dataLanguage));
	}
	buffListModel->addBuff(mainData->buffGroupHash["powercharm"], 1);
	buffListModel->addBuff(mainData->buffGroupHash["powertalon"], 1);
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

const BuffGroup *MainWindow::getBuffGroup() const {
	int idx = ui->buffCb->currentIndex();
	if (mainData && idx >= 0 && idx < mainData->buffGroups.count()) {
		return mainData->buffGroups[idx];
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
	buffListModel->setDataLanguage(dataLanguage);
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
	for (int idx = 0; idx < ui->buffCb->count(); ++idx) {
		if (mainData && idx < mainData->buffGroups.count()) {
			ui->buffCb->setItemText(idx, mainData->buffGroups[idx]->
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

void MainWindow::selectBuffGroupFromList(const QModelIndex &index) {
	if (!mainData) return;
	if (!index.isValid()) return;
	const BuffListModel *model =
		qobject_cast<const BuffListModel *>(index.model());
	if (!model) return;
	const BuffGroup *group = model->buffGroup(index);
	if (!group) return;
	for (int idx = 0; idx < ui->buffCb->count(); ++idx) {
		if (idx < mainData->buffGroups.count() &&
		    mainData->buffGroups[idx] == group) {
			ui->buffCb->setCurrentIndex(idx);
			break;
		}
	}
}

void MainWindow::updateBuffGroupFromListSelection() {
	QModelIndexList sel = ui->buffListView->selectionModel()->selectedIndexes();
	if (sel.count() == 1) selectBuffGroupFromList(sel[0]);
}

void MainWindow::buffGroupChanged(int new_idx) {
	if (!mainData) return;
	if (new_idx < 0 || new_idx >= ui->buffCb->count()) return;
	QModelIndex idx =
		buffListModel->buffGroupIndex(mainData->buffGroups[new_idx]);
	if (idx.isValid()) {
		ui->buffListView->selectionModel()->
			select(idx, QItemSelectionModel::ClearAndSelect);
	} else {
		ui->buffListView->selectionModel()->clearSelection();
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
		build->buffLevels = buffListModel->getBuffLevels();
		build->decorationSlots << getDecorationSlots();

		QVector<Item *> useful_items = build->listUsefulItems(mainData->items);
		if (ui->ignoreWeaponSlots->isChecked()) {
			QVector<Item *>::iterator it = useful_items.begin();
			while (it != useful_items.end()) {
				if ((*it)->buffRefs.isEmpty()) {
					it = useful_items.erase(it);
				} else {
					++it;
				}
			}
		}

		QVector<Build *> augmented_builds;
		augmented_builds << build;
		if (!ui->ignoreAugmentations->isChecked()) {
			build->fillWeaponAugmentations(&augmented_builds, useful_items);
		}

		QVector<Build *> all_builds;
		foreach(Build *b, augmented_builds) {
			all_builds << b;
			b->fillSlots(&all_builds, useful_items);
		}
		foreach(Build *b, all_builds) {
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

void MainWindow::addBuff() {
	const BuffGroup *bg = getBuffGroup();
	if (bg) {
		buffListModel->addBuff(bg, 1);
		QModelIndex idx = buffListModel->buffGroupIndex(bg);
		if (idx.isValid()) {
			ui->buffListView->setCurrentIndex(idx);
			ui->buffListView->scrollTo(idx);
		}
	}
}

void MainWindow::removeBuff() {
	const BuffGroup *bg = getBuffGroup();
	if (bg) {
		buffListModel->removeBuff(bg, 1);
		QModelIndex idx = buffListModel->buffGroupIndex(bg);
		if (idx.isValid()) {
			ui->buffListView->setCurrentIndex(idx);
			ui->buffListView->scrollTo(idx);
		}
	}
}
