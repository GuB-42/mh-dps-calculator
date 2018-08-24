#ifndef DetailsDialog_h_
#define DetailsDialog_h_

#include <QDialog>
#include "../NamedObject.h"

struct Profile;
struct Target;
struct BuildWithDps;

namespace Ui {
	class DetailsDialog;
}
class DetailsDialog : public QDialog {
	Q_OBJECT
public:
	DetailsDialog(const Profile *profile,
	              const Target *target,
	              QVector<BuildWithDps *> rd,
                  Language lang,
                  QWidget *parent = NULL);
	virtual ~DetailsDialog();

public slots:
	void setDataLanguage(Language lang);

protected:
	void changeEvent(QEvent * event);

private:
	void updateContent();

	Ui::DetailsDialog *ui;
	QVector<BuildWithDps *> resultData;
	Language dataLanguage;
};

#endif
