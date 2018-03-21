#include <QAbstractTableModel>
#include <QVector>
#include "../NamedObject.h"

class BuildWithDps;

class ResultTableModel : public QAbstractTableModel {
	Q_OBJECT
public:
	explicit ResultTableModel(QObject *parent = 0);
	~ResultTableModel();

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

	void setDataLanguage(NamedObject::Language lang);
	void setResultData(const QVector<BuildWithDps *> &d);
private:
	void clearData();
	QVector<BuildWithDps *> resultData;
	int itemColumns;
	NamedObject::Language dataLanguage;
};
