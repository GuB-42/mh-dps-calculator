#ifndef BuffGroupListModel_h_
#define BuffGroupListModel_h_

#include <QAbstractListModel>
#include <QList>
#include "../enums.h"

struct BuffGroup;

class BuffGroupListModel : public QAbstractListModel {
	Q_OBJECT
public:
	explicit BuffGroupListModel(QObject *parent = 0);

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

	void clear();
	void addBuffGroup(const BuffGroup *group);

	const BuffGroup *buffGroup(const QModelIndex &index) const;
	QModelIndex buffGroupIndex(const BuffGroup *group) const;

public slots:
	void setDataLanguage(Language lang);

private:
	QList<const BuffGroup *> listData;
	QMap<const BuffGroup *, int> listDataMap;
	Language dataLanguage;
};

#endif
