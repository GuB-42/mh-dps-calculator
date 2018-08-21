#ifndef BuffListModel_h_
#define BuffListModel_h_

#include <QAbstractListModel>
#include <QList>
#include "../enums.h"
#include "../Build.h"

struct BuffGroup;

class BuffListModel : public QAbstractListModel {
	Q_OBJECT
public:
	explicit BuffListModel(QObject *parent = 0);

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

	void clear();
	void addBuff(const BuffGroup *group, int level);
	void removeBuff(const BuffGroup *group, int level);

	QVector<BuffWithLevel> getBuffLevels() const;
	const BuffGroup *buffGroup(const QModelIndex &index) const;
	QModelIndex buffGroupIndex(const BuffGroup *group) const;

public slots:
	void setDataLanguage(Language lang);

private:
	QList<BuffWithLevel> listData;
	QMap<const BuffGroup *, int> listDataMap;
	Language dataLanguage;
};

#endif
