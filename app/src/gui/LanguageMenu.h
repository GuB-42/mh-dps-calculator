#ifndef LanguageMenu_h_
#define LanguageMenu_h_

#include <QMenu>
#include "../enums.h"

class LanguageActionWrapper : public QAction {
	Q_OBJECT
public:
	LanguageActionWrapper(Language v, QObject *parent = NULL);
	inline Language value() const { return m_value; }
	void updateText();
signals:
	void languageChanged(Language v);
private slots:
	void triggeredSlot(bool checked);
private:
	Language m_value;
};

class LanguageMenu : public QMenu {
	Q_OBJECT
public:
	explicit LanguageMenu(QWidget *parent = NULL);
public slots:
	void setLanguage(Language v);
signals:
	void languageChanged(Language v);
protected:
	void changeEvent(QEvent * event);
private:
	QList<LanguageActionWrapper *> LanguageActions;
};

#endif
