#ifndef BuffChooser_h_
#define BuffChooser_h_

#include <QWidget>
#include "../NamedObject.h"
#include "../enums.h"

class BuffGroup;
class BuffListModel;
class BuffGroupListModel;
class QModelIndex;

namespace Ui {
	class BuffChooser;
}
class BuffChooser : public QWidget {
	Q_OBJECT
public:
	explicit BuffChooser(QWidget *parent = NULL);

	BuffListModel *buffListModel() const {
		return m_buffListModel;
	}

	BuffGroupListModel *buffGroupListModel() const {
		return m_buffGroupListModel;
	}

public slots:
	void setDataLanguage(Language lang);

protected:
	void changeEvent(QEvent * event);

private slots:
	void updateBuffGroupFromListSelection();
	void buffGroupChanged(int new_idx);
	void addBuff();
	void removeBuff();

private:
	void selectBuffGroupFromList(const QModelIndex &index);
	const BuffGroup *getBuffGroup() const;

	Ui::BuffChooser *ui;
	BuffListModel *m_buffListModel;
	BuffGroupListModel *m_buffGroupListModel;
};

#endif
