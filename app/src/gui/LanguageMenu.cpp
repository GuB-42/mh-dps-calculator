#include "LanguageMenu.h"
#include "MainApplication.h"
#include <QEvent>
#include <QTranslator>
#include <QLibraryInfo>

LanguageActionWrapper::LanguageActionWrapper(Language v, QObject *parent) :
	QAction(parent), m_value(v)
{
	setCheckable(true);
	updateText();
	connect(this, SIGNAL(triggered(bool)), this, SLOT(triggeredSlot(bool)));
}

void LanguageActionWrapper::updateText()
{
	switch (m_value) {
	case LANG_FR:
		setText(tr("French"));
		break;
	case LANG_EN:
		setText(tr("English"));
		break;
	case LANG_JA:
		setText(tr("Japanese"));
		break;
	case LANG_COUNT:
		break;
	}
}

static void switchTranslatorQt(const QString& filename)
{
	MainApplication *main_app = qobject_cast<MainApplication *>(qApp);
	if (!main_app) return;
	QTranslator *trans = main_app->translator();
	qApp->removeTranslator(trans);
	if (trans->load(filename, QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
		qApp->installTranslator(trans);
	}
}
static void switchTranslator(const QString& filename)
{
	MainApplication *main_app = qobject_cast<MainApplication *>(qApp);
	if (!main_app) return;
	QTranslator *trans = main_app->translator();
	qApp->removeTranslator(trans);
	if (trans->load(filename)) {
		qApp->installTranslator(trans);
	}
}

void LanguageActionWrapper::triggeredSlot(bool checked) {
	if (checked) {
		switch (m_value) {
		case LANG_FR:
			switchTranslatorQt("qt_fr.qm");
			switchTranslator(":/translations/fr.qm");
			break;
		case LANG_EN:
			switchTranslatorQt("qt_en.qm");
			switchTranslator(":/translations/en.qm");
			break;
		case LANG_JA:
			switchTranslatorQt("qt_ja.qm");
			switchTranslator(":/translations/ja.qm");
			break;
		case LANG_COUNT:
			break;
		}
		emit languageChanged(m_value);
	}
}

LanguageMenu::LanguageMenu(QWidget *parent) : QMenu(parent) {
	QActionGroup *group = new QActionGroup(this);
	for (int i = 0; i < LANG_COUNT; ++i) {
		LanguageActionWrapper *act =
			new LanguageActionWrapper((Language)i, group);
		group->addAction(act);
		LanguageActions.append(act);
		connect(act, SIGNAL(languageChanged(Language)),
		        this, SIGNAL(languageChanged(Language)));
		group->addAction(act);
		addAction(act);
	}
}

void LanguageMenu::setLanguage(Language v) {
	if (v >= 0 && v < LANG_COUNT) {
		LanguageActions[v]->setChecked(true);
	}
}

void LanguageMenu::changeEvent(QEvent *event) {
	if (event) {
		switch (event->type()) {
		case QEvent::LanguageChange:
			foreach (LanguageActionWrapper *act, LanguageActions) {
				act->updateText();
			}
			break;
		default:
			break;
		}
	}
	QMenu::changeEvent(event);
}
