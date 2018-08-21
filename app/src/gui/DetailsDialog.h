#ifndef DetailsDialog_h_
#define DetailsDialog_h_

#include <QDialog>
#include "../NamedObject.h"

struct BuildWithDps;
struct BuffWithCondition;
struct Damage;
struct Dps;
struct FoldedBuffsData;
struct Weapon;

namespace Ui {
	class DetailsDialog;
}
class DetailsDialog : public QDialog {
	Q_OBJECT
public:
	DetailsDialog(QVector<BuildWithDps *> rd,
                  Language lang,
                  QWidget *parent = NULL);
	virtual ~DetailsDialog();

public slots:
	void setDataLanguage(Language lang);

protected:
	void changeEvent(QEvent * event);

private:
	static QString weaponTableHtml(const Weapon &weapon);
	static QString buffLineHtml(const BuffWithCondition &bwc);
	static QString buffTableHtml(const QVector<const BuffWithCondition *> &bwcs);
	static QString damageTableHeaderHtml(const Damage &damage,
	                                     QString header1, QString header2);
	static QString damageTableHtml(const Damage &damage);
	static QString foldedBuffLineHtml(const FoldedBuffsData &buff, int row);
	static QString damageParamsTableHtml(const Damage &damage);
	static QString foldedBuffsTableHtml(const Damage &damage);
	static QString dpsTableHtml(const Dps &dps);
	static QString statusProcTableHtml(const Dps &dps);
	static QString dpsParamsTableHtml(const Dps &dps);
	QString singleDataToHtml(BuildWithDps *bwd) const;
	QString dataToHtml(QVector<BuildWithDps *> bwds) const;
	void updateContent();

	Ui::DetailsDialog *ui;
	QVector<BuildWithDps *> resultData;
	Language dataLanguage;
};

#endif
