#ifndef ComputeDataModel_h_
#define ComputeDataModel_h_

#include <QAbstractTableModel>
#include "../ConditionRatios.h"

struct Profile;
struct Target;

class ComputeDataModel : public QAbstractTableModel {
	Q_OBJECT
public:
	explicit ComputeDataModel(const Profile *p, const Target *t,
	                          QObject *parent = 0);

	static int conditionRow(Condition c);
	static Condition rowCondition(int row);

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

private:
	ConditionRatios ratios;
	double sharpenPeriod;
	double defenseMultiplier;
	double statusDefenseMultiplier;
	double statusHitMultiplier;
};

#endif
