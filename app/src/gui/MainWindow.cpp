#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QClipboard>
#include <QProgressBar>
#include <QLibraryInfo>

#include "ResultTableModel.h"
#include "BuffListModel.h"
#include "BuffGroupListModel.h"
#include "Computer.h"
#include "GeneticComputer.h"
#include "ComputeDataDialog.h"
#include "DetailsDialog.h"
#include "../MainData.h"
#include "../Target.h"
#include "../Profile.h"
#include "../BuffGroup.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	progressBar(new QProgressBar(this)),
	mainData(NULL),
	tableModel(new ResultTableModel(this)),
	buffListModel(new BuffListModel(this)),
	buffGroupListModel(new BuffGroupListModel(this)),
	dataLanguage(LANG_EN),
	computer(new Computer(this)),
	geneticComputer(new GeneticComputer(this))
{
	ui->setupUi(this);

	progressBar->setSizePolicy(QSizePolicy::Expanding,
	                           QSizePolicy::Preferred);
	ui->statusbar->addWidget(progressBar);

	for (int i = 0; i < LANG_COUNT; ++i) {
		ui->languageCb->addItem(toString((Language)i));
	}
	connect(ui->languageCb, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(changeLanguage(int)));

	tableModel->setDataLanguage(dataLanguage);
	connect(this, SIGNAL(dataLanguageChanged(Language)),
	        tableModel, SLOT(setDataLanguage(Language)));
	ui->tableView->setModel(tableModel);
	ui->tableView->addAction(ui->action_Copy);
	ui->tableView->addAction(ui->action_ShowDetails);

	for (int i = 0; i < ResultTableModel::COLUMN_COUNT; ++i) {
		if (tableModel->isDefaultVisibleColumn((ResultTableModel::Column)i)) {
			ui->tableView->showColumn(i);
		} else {
			ui->tableView->hideColumn(i);
		}
	}
	ui->menu_Columns->setView(ui->tableView);
	ui->menu_MonsterMode->setValue(MODE_ENRAGED_WEAK_SPOT);
	tableModel->setMonsterMode(MODE_ENRAGED_WEAK_SPOT);
	connect(ui->menu_MonsterMode, SIGNAL(valueSelected(MonsterMode)),
	        tableModel, SLOT(setMonsterMode(MonsterMode)));

	buffListModel->setDataLanguage(dataLanguage);
	connect(this, SIGNAL(dataLanguageChanged(Language)),
	        buffListModel, SLOT(setDataLanguage(Language)));
	ui->buffListView->setModel(buffListModel);

	buffGroupListModel->setDataLanguage(dataLanguage);
	connect(this, SIGNAL(dataLanguageChanged(Language)),
	        buffGroupListModel, SLOT(setDataLanguage(Language)));
	ui->buffCb->setModel(buffGroupListModel);

	connect(ui->buffListView->selectionModel(),
	        SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
	        this, SLOT(updateBuffGroupFromListSelection()));
	connect(ui->buffCb, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(buffGroupChanged(int)));

	connect(ui->calculateDps, SIGNAL(clicked()), this, SLOT(calculate()));
	connect(ui->action_Quit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui->action_Copy, SIGNAL(triggered()), this, SLOT(copy()));
	connect(ui->action_ShowParameters, SIGNAL(triggered()), this, SLOT(showParameters()));
	connect(ui->action_ShowDetails, SIGNAL(triggered()), this, SLOT(showDetails()));

	connect(ui->addBuff, SIGNAL(clicked()), this, SLOT(addBuff()));
	connect(ui->removeBuff, SIGNAL(clicked()), this, SLOT(removeBuff()));

	connect(QApplication::instance(), SIGNAL(focusChanged(QWidget*, QWidget*)),
	        this, SLOT(updateCopyAction()));
	connect(ui->tableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
	        this, SLOT(updateCopyAction()));
	updateCopyAction();

#if QT_VERSION >= 0x050000
	ui->tableView->horizontalHeader()->setSectionsMovable(true);
#else
	ui->tableView->horizontalHeader()->setMovable(true);
#endif
	connect(ui->tableView->horizontalHeader(), SIGNAL(sectionMoved(int, int, int)),
	        this, SLOT(updateTableMimeColumnOrder()));
	updateTableMimeColumnOrder();

	ui->splitter->setSizes(QList<int>() << 1 << height());

	connect(computer, SIGNAL(progress(int, int, int)),
	        this, SLOT(calculationProgress(int, int, int)));
	connect(computer, SIGNAL(finished(const QVector<BuildWithDps *> &)),
	        this, SLOT(calculationFinished(const QVector<BuildWithDps *> &)));
	connect(geneticComputer, SIGNAL(progress(int, int, int)),
	        this, SLOT(calculationProgress(int, int, int)));
	connect(geneticComputer, SIGNAL(finished(const QVector<BuildWithDps *> &)),
	        this, SLOT(calculationFinished(const QVector<BuildWithDps *> &)));
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
	for (int i = 0; i < ui->level4Slots->value(); ++i) ret.append(4);
	for (int i = 0; i < ui->level3Slots->value(); ++i) ret.append(3);
	for (int i = 0; i < ui->level2Slots->value(); ++i) ret.append(2);
	for (int i = 0; i < ui->level1Slots->value(); ++i) ret.append(1);
	return ret;
}

QVector<int> MainWindow::getUsedSlots() const {
	QVector<int> ret;
	for (int i = 0; i < -ui->level4Slots->value(); ++i) ret.append(4);
	for (int i = 0; i < -ui->level3Slots->value(); ++i) ret.append(3);
	for (int i = 0; i < -ui->level2Slots->value(); ++i) ret.append(2);
	for (int i = 0; i < -ui->level1Slots->value(); ++i) ret.append(1);
	return ret;
}

static void switchTranslatorQt(QTranslator& translator, const QString& filename)
{
	qApp->removeTranslator(&translator);
	if (translator.load(filename, QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
		qApp->installTranslator(&translator);
	}
}
static void switchTranslator(QTranslator& translator, const QString& filename)
{
	qApp->removeTranslator(&translator);
	if (translator.load(filename)) {
		qApp->installTranslator(&translator);
	}
}

void MainWindow::setDataLanguage(Language lang) {
	switch (lang) {
	case LANG_FR:
		switchTranslatorQt(translatorQt, "qt_fr.qm");
		switchTranslator(translator, ":/translations/fr.qm");
		break;
	case LANG_EN:
		switchTranslatorQt(translatorQt, "qt_en.qm");
		switchTranslator(translator, ":/translations/en.qm");
		break;
	case LANG_JP:
		switchTranslatorQt(translatorQt, "qt_ja.qm");
		switchTranslator(translator, ":/translations/ja.qm");
		break;
	case LANG_COUNT:
		break;
	}

	dataLanguage = lang;
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
	emit dataLanguageChanged(lang);
}

void MainWindow::updateCopyAction() {
	if (ui->tableView->selectionModel()->hasSelection()) {
		ui->action_Copy->setEnabled(true);
		ui->action_ShowDetails->setEnabled(true);
	} else {
		ui->action_Copy->setEnabled(false);
		ui->action_ShowDetails->setEnabled(false);
	}
}

void MainWindow::changeLanguage(int lang_idx) {
	if (lang_idx >= 0 && lang_idx < LANG_COUNT) {
		setDataLanguage((Language)lang_idx);
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

void MainWindow::calculate() {
	if (mainData && getProfile() && getTarget()) {
		if (ui->geneticMode->isChecked()) {
			GeneticComputer::Parameters params;
			params.profile = getProfile();
			params.target = getTarget();
			params.weapons = mainData->weapons;
			params.items = mainData->items;
			params.buffLevels = buffListModel->getBuffLevels();
			params.decorationSlots = getDecorationSlots();
			params.usedSlots = getUsedSlots();
			params.ignoreAugmentations = ui->ignoreAugmentations->isChecked();
			params.ignoreWeaponSlots = ui->ignoreWeaponSlots->isChecked();
			params.finalOnly = ui->finalOnly->isChecked();
			tableModel->clear();
			geneticComputer->compute(params);
		} else {
			Computer::Parameters params;
			params.profile = getProfile();
			params.target = getTarget();
			params.weapons = mainData->weapons;
			params.items = mainData->items;
			params.buffLevels = buffListModel->getBuffLevels();
			params.decorationSlots = getDecorationSlots();
			params.usedSlots = getUsedSlots();
			params.ignoreAugmentations = ui->ignoreAugmentations->isChecked();
			params.ignoreWeaponSlots = ui->ignoreWeaponSlots->isChecked();
			params.finalOnly = ui->finalOnly->isChecked();
			tableModel->clear();
			computer->compute(params);
		}
	}
}

void MainWindow::calculationProgress(int min, int max, int value) {
	progressBar->setRange(min, max);
	progressBar->setValue(value);
}

void MainWindow::calculationFinished(const QVector<BuildWithDps *> &data) {
	tableModel->setResultData(data);
	ui->tableView->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
}

void MainWindow::updateTableMimeColumnOrder() {
	QList<int> order;
	for (int i = 0; i < tableModel->columnCount(); ++i) {
		int logi = ui->tableView->horizontalHeader()->logicalIndex(i);
		if (i != logi) {
			while (order.count() < i - 1) order.append(order.count());
			order.append(logi);
		}
	}
	tableModel->setMimeColumnOrder(order);
}

void MainWindow::copy() {
	if (ui->tableView->selectionModel()) {
		QModelIndexList indexes;
		foreach(const QModelIndex &index,
		        ui->tableView->selectionModel()->selectedIndexes()) {
			if (index.isValid() && index.model() == ui->tableView->model() &&
			    !ui->tableView->isColumnHidden(index.column())) {
				indexes.append(index);
			}
		}
		QApplication::clipboard()->
			setMimeData(ui->tableView->model()->mimeData(indexes));
	}
}

void MainWindow::showParameters() {
	if (mainData && getProfile() && getTarget()) {
		ComputeDataDialog *cdd =
			new ComputeDataDialog(getProfile(), getTarget(), this);
		cdd->show();
	}
}

void MainWindow::showDetails() {
	if (ui->tableView->selectionModel()) {
		QModelIndexList indexes;
		foreach(const QModelIndex &index,
		        ui->tableView->selectionModel()->selectedIndexes()) {
			if (index.isValid() && index.model() == tableModel) {
				indexes.append(index);
			}
		}
		DetailsDialog *dd =
			new DetailsDialog(getProfile(), getTarget(),
			                  tableModel->resultDataList(indexes),
			                  dataLanguage, this);
		connect(this, SIGNAL(dataLanguageChanged(Language)),
		        dd, SLOT(setDataLanguage(Language)));
		dd->show();
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

void MainWindow::changeEvent(QEvent *event) {
	if (event) {
		switch (event->type()) {
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;
		default:
			break;
		}
	}
	QMainWindow::changeEvent(event);
}
