#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QClipboard>
#include <QProgressBar>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrentMap>

#include "ResultTableModel.h"
#include "BuffListModel.h"
#include "BuffGroupListModel.h"
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
	progressBar(new QProgressBar(this)),
	mainData(NULL),
	tableModel(new ResultTableModel(this)),
	buffListModel(new BuffListModel(this)),
	buffGroupListModel(new BuffGroupListModel(this)),
	dataLanguage(NamedObject::LANG_EN)
{
	ui->setupUi(this);

	progressBar->setSizePolicy(QSizePolicy::Expanding,
	                           QSizePolicy::Preferred);
	ui->statusbar->addWidget(progressBar);

	for (int i = 0; i < NamedObject::LANG_COUNT; ++i) {
		ui->languageCb->addItem(toString((NamedObject::Language)i));
	}
	connect(ui->languageCb, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(changeLanguage(int)));

	tableModel->setDataLanguage(dataLanguage);
	ui->tableView->setModel(tableModel);
	ui->tableView->addAction(ui->action_Copy);
	ui->tableView->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);

	buffListModel->setDataLanguage(dataLanguage);
	ui->buffListView->setModel(buffListModel);

	buffGroupListModel->setDataLanguage(dataLanguage);
	ui->buffCb->setModel(buffGroupListModel);

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
	tableModel->clear();
	buffListModel->clear();
	buffGroupListModel->clear();
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
	foreach(BuffGroup *buff_group, mainData->buffGroups) {
		if (buff_group->hasBuffs()) {
			buffGroupListModel->addBuffGroup(buff_group);
		}
	}

	QStringList base_buff_names;
	base_buff_names << "powercharm" << "powertalon";
	foreach(QString base_buff_name, base_buff_names) {
		const BuffGroup *bg =
			mainData->buffGroupHash[base_buff_name];
		if (bg) buffListModel->addBuff(bg, 1);
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

const BuffGroup *MainWindow::getBuffGroup() const {
	int idx = ui->buffCb->currentIndex();

	if (idx >= 0) {
		const BuffGroupListModel *model =
			qobject_cast<const BuffGroupListModel *>(ui->buffCb->model());
		if (model) return model->buffGroup(model->index(idx));
	}
	return NULL;
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
	buffGroupListModel->setDataLanguage(dataLanguage);
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

void MainWindow::selectBuffGroupFromList(const QModelIndex &index) {
	if (!mainData) return;
	if (!index.isValid()) return;
	const BuffGroup *group = NULL;
	{
		const BuffListModel *model =
			qobject_cast<const BuffListModel *>(index.model());
		if (model) group = model->buffGroup(index);
	}
	if (group) {
		const BuffGroupListModel *model =
			qobject_cast<const BuffGroupListModel *>(ui->buffCb->model());
		if (model) {
			QModelIndex idx = model->buffGroupIndex(group);
			if (idx.isValid()) {
				ui->buffCb->setCurrentIndex(idx.row());
			}
		}
	}
}

void MainWindow::updateBuffGroupFromListSelection() {
	QModelIndexList sel = ui->buffListView->selectionModel()->selectedIndexes();
	if (sel.count() == 1) selectBuffGroupFromList(sel[0]);
}

void MainWindow::buffGroupChanged(int new_idx) {
	const BuffGroup *group = NULL;
	if (new_idx >= 0) {
		const BuffGroupListModel *model =
			qobject_cast<const BuffGroupListModel *>(ui->buffCb->model());
		if (model) group = model->buffGroup(model->index(new_idx));
	}
	if (group) {
		QModelIndex idx = buffListModel->buffGroupIndex(group);
		if (idx.isValid()) {
			ui->buffListView->selectionModel()->
				select(idx, QItemSelectionModel::ClearAndSelect);
		} else {
			ui->buffListView->selectionModel()->clearSelection();
		}
	}
}

struct MakeBuilds
{
	explicit MakeBuilds(bool ia) : ignoreAugmentations(ia) {};
	void operator()(MainWindow::BuildFutureElt &elt) {
		QVector<Build *> augmented_builds;
		foreach(Build *b, elt.builds) {
			augmented_builds << b;
			if (!ignoreAugmentations) {
				b->fillWeaponAugmentations(&augmented_builds, elt.useful_items);
			}
		}
		elt.builds.clear();
		foreach(Build *b, augmented_builds) {
			elt.builds << b;
			b->fillSlots(&elt.builds, elt.useful_items);
		}
	}
	bool ignoreAugmentations;
};

void MainWindow::calculate() {
	const Profile *profile = getProfile();
	const Target *target = getTarget();

	if (!profile || !target) return;

	for (int i = 0; i < buildFutures.count(); ++i) {
		buildFutures[i].future.cancel();
	}
	for (int i = 0; i < resultFutures.count(); ++i) {
		resultFutures[i].future.cancel();
	}
	tableModel->clear();

	BuildFuture new_future(profile, target);
	foreach(const Weapon *weapon, mainData->weapons) {
		if (weapon->type != profile->type) continue;
		if (!weapon->final && ui->finalOnly->isChecked()) continue;

		BuildFutureElt elt;
		Build *build = new Build;
		build->addWeapon(weapon);
		if (ui->ignoreWeaponSlots->isChecked()) {
			build->decorationSlots.clear();
		}
		build->buffLevels = buffListModel->getBuffLevels();
		build->decorationSlots << getDecorationSlots();
		elt.useful_items = build->listUsefulItems(mainData->items);
		if (ui->ignoreWeaponSlots->isChecked()) {
			QVector<Item *>::iterator it = elt.useful_items.begin();
			while (it != elt.useful_items.end()) {
				if ((*it)->buffRefs.isEmpty()) {
					it = elt.useful_items.erase(it);
				} else {
					++it;
				}
			}
		}
		elt.builds.append(build);
		new_future.data.append(elt);
	}

	new_future.future =
		QtConcurrent::map(new_future.data,
		                  MakeBuilds(ui->ignoreAugmentations->isChecked()));
	buildFutures.append(new_future);
	QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
	watcher->setFuture(new_future.future);
	connect(watcher, SIGNAL(finished()), this, SLOT(buildFutureFinished()));
	connect(watcher, SIGNAL(progressValueChanged(int)), this, SLOT(buildFutureProgress(int)));
	connect(watcher, SIGNAL(finished()), watcher, SLOT(deleteLater()));
}

struct MakeResult
{
	MakeResult(const Profile &p, const Target &t) :
		profile(p), target(t) {};
	void operator()(BuildWithDps *bwd) {
		bwd->compute(profile, target);
	}
	const Profile &profile;
	const Target &target;
};

void MainWindow::buildFutureFinished() {
	while (!buildFutures.isEmpty()) {
		BuildFuture &fdata = buildFutures.first();
		if (!fdata.future.isFinished()) break;
		if (buildFutures.count() == 1 && !fdata.future.isCanceled()) {
			ResultFuture new_future;
			foreach(const BuildFutureElt &elt, fdata.data) {
				foreach(Build *b, elt.builds) {
					new_future.result.append(new BuildWithDps(b));
				}
			}
			new_future.result.squeeze();
			new_future.future =
				QtConcurrent::map(new_future.result,
			                      MakeResult(*fdata.profile, *fdata.target));
			resultFutures.append(new_future);
			QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
			watcher->setFuture(new_future.future);
			connect(watcher, SIGNAL(finished()), this, SLOT(resultFutureFinished()));
			connect(watcher, SIGNAL(progressValueChanged(int)), this, SLOT(resultFutureProgress(int)));
			connect(watcher, SIGNAL(finished()), watcher, SLOT(deleteLater()));
		} else {
			foreach(const BuildFutureElt &elt, fdata.data) {
				foreach(Build *b, elt.builds) delete b;
			}
		}
		buildFutures.removeFirst();
	}
}

void MainWindow::buildFutureProgress(int value) {
	if (!buildFutures.isEmpty()) {
		int min = buildFutures.last().future.progressMinimum();
		int max = buildFutures.last().future.progressMaximum();
		int offset = (max - min) * 5;
		progressBar->setRange(min, max + offset);
		progressBar->setValue(value);
	}
}

void MainWindow::resultFutureFinished() {
	while (!resultFutures.isEmpty()) {
		ResultFuture &fdata = resultFutures.first();
		if (!fdata.future.isFinished()) break;
		if (resultFutures.count() == 1 && !fdata.future.isCanceled()) {
			tableModel->setResultData(fdata.result);
			ui->tableView->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
		} else {
			foreach(BuildWithDps *bwd, fdata.result) delete bwd;
		}
		resultFutures.removeFirst();
	}
}

void MainWindow::resultFutureProgress(int value) {
	if (!resultFutures.isEmpty()) {
		int min = resultFutures.last().future.progressMinimum();
		int max = resultFutures.last().future.progressMaximum();
		int offset = (max - min) / 5;
		progressBar->setRange(min, max + offset);
		progressBar->setValue(value + offset);
	}
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
