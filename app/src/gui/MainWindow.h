#ifndef MainWindow_h_
#define MainWindow_h_

#include <QMainWindow>
#include "../NamedObject.h"

class ResultTableModel;
class MainData;
class Profile;
class Target;

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
	void setDataLanguage(NamedObject::Language lang);

public slots:
	void calculate();

private slots:
	void changeLanguage(int lang_idx);

private:
	Ui::MainWindow *ui;
	const MainData *mainData;

	ResultTableModel *tableModel;
	NamedObject::Language dataLanguage;
};

#endif