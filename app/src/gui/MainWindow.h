#ifndef MainWindow_h_
#define MainWindow_h_

#include <QMainWindow>
#include <QTranslator>
#include "../NamedObject.h"

class QModelIndex;
class QProgressBar;
class ResultTableModel;
class BuffListModel;
class BuffGroupListModel;
class Computer;
class GeneticComputer;
struct MainData;
struct Profile;
struct Target;
struct BuffGroup;
struct BuildWithDps;

namespace Ui {
	class MainWindow;
}
class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = NULL);
	virtual ~MainWindow();

	void clearAll();
	void setMainData(const MainData *md);
	const Profile *getProfile() const;
	const Target *getTarget() const;

	QVector<int> getDecorationSlots() const;
	QVector<int> getUsedSlots() const;

public slots:
	void setDataLanguage(Language lang);
	void calculate();
	void calculationProgress(int min, int max, int value);
	void calculationFinished(const QVector<BuildWithDps *> &data);
	void copy();
	void showParameters();
	void showDetails();

signals:
	void dataLanguageChanged(Language lang);

private slots:
	void updateCopyAction();
	void updateTableMimeColumnOrder();

protected:
	void changeEvent(QEvent * event);

private:
	Ui::MainWindow *ui;

	QProgressBar *progressBar;
	const MainData *mainData;

	ResultTableModel *tableModel;
	Language dataLanguage;

	Computer *computer;
	GeneticComputer *geneticComputer;
};

#endif
