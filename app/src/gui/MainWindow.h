#ifndef MainWindow_h_
#define MainWindow_h_

#include <QMainWindow>
#include "../NamedObject.h"

class QModelIndex;
class ResultTableModel;
class BuffListModel;
struct MainData;
struct Profile;
struct Target;
struct BuffGroup;

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
	void copy();
	void addBuff();
	void removeBuff();

private slots:
	void updateCopyAction();
	void changeLanguage(int lang_idx);
	void selectBuffGroupFromList(const QModelIndex &index);
	void updateBuffGroupFromListSelection();
	void buffGroupChanged(int new_idx);

private:
	Ui::MainWindow *ui;
	const MainData *mainData;

	ResultTableModel *tableModel;
	BuffListModel *buffListModel;
	NamedObject::Language dataLanguage;
};

#endif
