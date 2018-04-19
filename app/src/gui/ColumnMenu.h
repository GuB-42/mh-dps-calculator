#ifndef ColumnMenu_h_
#define ColumnMenu_h_

#include <QMenu>

class QTableView;
class QModelIndex;
class QAbstractItemModel;

class ColumnActionWrapper : public QAction {
	Q_OBJECT
public:
	ColumnActionWrapper(int column, QObject *parent = NULL);
	inline int column() const { return m_column; }
	inline void setColumn(int col) { m_column = col; };
signals:
	void hideColumn(int column);
	void showColumn(int column);
private slots:
	void triggeredSlot(bool checked);
private:
	int m_column;
};

class ColumnMenu : public QMenu {
	Q_OBJECT
public:
	explicit ColumnMenu(QWidget *parent = NULL);
	void setView(QTableView *t);
private slots:
	void rebuildMenu();
	void viewDestroyed();
	void modelDestroyed();
	void columnsInserted(const QModelIndex &, int first, int last);
	void columnsRemoved(const QModelIndex &, int first, int last);
	void headerDataChanged(Qt::Orientation orientation, int first, int last);
	void columnsMoved(const QModelIndex &, int start, int end,
                      const QModelIndex &, int column);
private:
	void clearMenu();

	QTableView *view;
	QAbstractItemModel *model;
	QList<ColumnActionWrapper *> columnMenuActions;
};

#endif
