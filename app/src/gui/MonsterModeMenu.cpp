#include "MonsterModeMenu.h"

MonsterModeActionWrapper::MonsterModeActionWrapper(MonsterMode v, QObject *parent) :
	QAction(parent), m_value(v)
{
	setCheckable(true);
	updateText();
	connect(this, SIGNAL(triggered(bool)), this, SLOT(triggeredSlot(bool)));
}

void MonsterModeActionWrapper::updateText()
{
	switch (m_value) {
	case MODE_NORMAL_NORMAL:
		setText(tr("Normal"));
		break;
	case MODE_NORMAL_WEAK_SPOT:
		setText(tr("Weak spot"));
		break;
	case MODE_ENRAGED_NORMAL:
		setText(tr("Enraged"));
		break;
	case MODE_ENRAGED_WEAK_SPOT:
		setText(tr("Weak spot / Enraged"));
		break;
	case MODE_COUNT:
		break;
	}
}

void MonsterModeActionWrapper::triggeredSlot(bool checked) {
	if (checked) emit valueSelected(m_value);
}

MonsterModeMenu::MonsterModeMenu(QWidget *parent) : QMenu(parent) {
	QActionGroup *group = new QActionGroup(this);
	for (int i = 0; i < MODE_COUNT; ++i) {
		MonsterModeActionWrapper *act =
			new MonsterModeActionWrapper((MonsterMode)i, group);
		group->addAction(act);
		monsterModeActions.append(act);
		connect(act, SIGNAL(valueSelected(MonsterMode)),
		        this, SIGNAL(valueSelected(MonsterMode)));
		group->addAction(act);
		addAction(act);
	}
}

void MonsterModeMenu::setValue(MonsterMode v) {
	if (v >= 0 && v < monsterModeActions.count()) {
		monsterModeActions[v]->setChecked(true);
	}
}
