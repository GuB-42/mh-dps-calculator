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
	const BuffGroup *getBuffGroup() const;

	QVector<int> getDecorationSlots() const;
	void setDataLanguage(NamedObject::Language lang);

public slots:
	void calculate();
	void calculationProgress(int min, int max, int value);
	void calculationFinished(const QVector<BuildWithDps *> &data);
	void copy();
	void showParameters();
	void addBuff();
	void removeBuff();

private slots:
	void updateCopyAction();
	void changeLanguage(int lang_idx);
	void selectBuffGroupFromList(const QModelIndex &index);
	void updateBuffGroupFromListSelection();
	void buffGroupChanged(int new_idx);
	void updateTableMimeColumnOrder();

protected:
	void changeEvent(QEvent * event);

private:
	Ui::MainWindow *ui;
	QTranslator translator;
	QTranslator translatorQt;

	QProgressBar *progressBar;
	const MainData *mainData;

	ResultTableModel *tableModel;
	BuffListModel *buffListModel;
	BuffGroupListModel *buffGroupListModel;
	NamedObject::Language dataLanguage;

	Computer *computer;
};

#endif
