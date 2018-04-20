#ifndef MonsterModeMenu_h_
#define MonsterModeMenu_h_

#include <QMenu>
#include "../enums.h"

class MonsterModeActionWrapper : public QAction {
	Q_OBJECT
public:
	MonsterModeActionWrapper(MonsterMode v, QObject *parent = NULL);
	inline MonsterMode value() const { return m_value; }
	void updateText();
signals:
	void valueSelected(MonsterMode v);
private slots:
	void triggeredSlot(bool checked);
private:
	MonsterMode m_value;
};

class MonsterModeMenu : public QMenu {
	Q_OBJECT
public:
	explicit MonsterModeMenu(QWidget *parent = NULL);
public slots:
	void setValue(MonsterMode v);
signals:
	void valueSelected(MonsterMode v);
protected:
	void changeEvent(QEvent * event);
private:
	QList<MonsterModeActionWrapper *> monsterModeActions;
};

#endif
