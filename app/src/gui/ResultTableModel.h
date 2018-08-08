#ifndef ResultTableModel_h_
#define ResultTableModel_h_

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
		COLUMN_KILL_TIME,
		COLUMN_POISON_PROC_RATE,
		COLUMN_PARALYSIS_PROC_RATE,
		COLUMN_SLEEP_PROC_RATE,
		COLUMN_STUN_PROC_RATE,
		COLUMN_BLAST_PROC_RATE,
		COLUMN_EXHAUST_PROC_RATE,
		COLUMN_MOUNT_PROC_RATE,
		COLUMN_BOUNCE_RATE,
		COLUMN_CRIT_RATE,
		COLUMN_SHARPNESS_USE,
		COLUMN_DMG_CUT_PIERCING,
		COLUMN_DMG_IMPACT,
		COLUMN_DMG_FIXED,
		COLUMN_DMG_ELEMENT,
		COLUMN_DMG_STATUS,
		COLUMN_DMG_STUN,
		COLUMN_DMG_EXHAUST,
		COLUMN_DMG_MOUNT,
		COLUMN_WEAPON_NAME,
		COLUMN_COUNT
	};

	explicit ResultTableModel(QObject *parent = 0);
	~ResultTableModel();

	static bool isNumberColumn(Column c);
	static bool isGreaterIsBetterColumn(Column c);
	static bool isDefaultVisibleColumn(Column c);
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
	void setMimeColumnOrder(QList<int> order);

public slots:
	void setMonsterMode(MonsterMode mode);

private:
	QVector<QVector<QVariant> > getDataTable(const QModelIndexList &indexes) const;
	void clearData();

	QVector<BuildWithDps *> resultData;
	int itemColumns;
	QList<int> mimeColumnOrder;
	NamedObject::Language dataLanguage;
	MonsterMode monsterMode;
};

#endif
