#ifndef ComputeDataDialog_h_
#define ComputeDataDialog_h_

#include <QDialog>

class ComputeDataModel;
struct Profile;
struct Target;

namespace Ui {
	class ComputeDataDialog;
}
class ComputeDataDialog : public QDialog {
	Q_OBJECT
public:
	ComputeDataDialog(const Profile *p, const Target *t,
	                  QWidget *parent = NULL);
	virtual ~ComputeDataDialog();

protected:
	void changeEvent(QEvent * event);

private:
	Ui::ComputeDataDialog *ui;
	ComputeDataModel *model;
};

#endif
