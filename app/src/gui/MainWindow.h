#ifndef MainWindow_h_
#define MainWindow_h_

#include <QMainWindow>
#include <QFuture>
#include "../NamedObject.h"

class QModelIndex;
class QProgressBar;
class ResultTableModel;
class BuffListModel;
class BuffGroupListModel;
struct MainData;
struct Profile;
struct Target;
struct BuffGroup;
struct Build;
struct Item;
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
	void copy();
	void addBuff();
	void removeBuff();

private slots:
	void updateCopyAction();
	void changeLanguage(int lang_idx);
	void selectBuffGroupFromList(const QModelIndex &index);
	void updateBuffGroupFromListSelection();
	void buffGroupChanged(int new_idx);
	void buildFutureFinished();
	void buildFutureProgress(int value);
	void resultFutureFinished();
	void resultFutureProgress(int value);

public:
	struct BuildFutureElt {
		QVector<Build *> builds;
		QVector<Item *> useful_items;
	};
	struct BuildFuture {
		BuildFuture() : profile(NULL), target(NULL) { }
		BuildFuture(const Profile *p, const Target *t) :
			profile(p), target(t) { }
		const Profile *profile;
		const Target *target;
		QVector<BuildFutureElt> data;
		QFuture<void> future;
	};
	struct ResultFuture {
		QVector<BuildWithDps *> result;
		QFuture<void> future;
	};

private:
	Ui::MainWindow *ui;
	QProgressBar *progressBar;
	const MainData *mainData;

	ResultTableModel *tableModel;
	BuffListModel *buffListModel;
	BuffGroupListModel *buffGroupListModel;
	NamedObject::Language dataLanguage;

	QList<BuildFuture> buildFutures;
	QList<ResultFuture> resultFutures;
};

#endif
