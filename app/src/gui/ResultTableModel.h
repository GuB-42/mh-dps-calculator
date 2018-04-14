#include <QAbstractTableModel>
#include <QVector>
#include <QColor>
#include "../NamedObject.h"
#include "../enums.h"

struct BuildWithDps;

class ResultTableModel : public QAbstractTableModel {
	Q_OBJECT
public:
	enum Column {
		COLUMN_TOTAL_DPS,
		COLUMN_RAW_DPS,
		COLUMN_ELEMENT_DPS,
		COLUMN_STATUS_DPS,
		COLUMN_FIXED_DPS,
		COLUMN_BOUNCE_RATE,
		COLUMN_SHARPNESS_USE,
		COLUMN_WEAPON_NAME,
		COLUMN_COUNT
	};

	explicit ResultTableModel(QObject *parent = 0);
	~ResultTableModel();

	static QString columnName(Column c);

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	QMimeData *mimeData(const QModelIndexList &indexes) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

	void setDataLanguage(NamedObject::Language lang);
	void setResultData(const QVector<BuildWithDps *> &d);
	void clear();

private:
	QVector<QVector<QVariant> > getDataTable(const QModelIndexList &indexes) const;
	void clearData();

	QVector<BuildWithDps *> resultData;
	int itemColumns;
	NamedObject::Language dataLanguage;
};
