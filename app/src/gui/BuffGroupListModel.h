#ifndef BuffGroupListModel_h_
#define BuffGroupListModel_h_

#include <QAbstractListModel>
#include <QList>
#include "../NamedObject.h"

struct BuffGroup;

class BuffGroupListModel : public QAbstractListModel {
	Q_OBJECT
public:
	explicit BuffGroupListModel(QObject *parent = 0);

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

	void clear();
	void addBuffGroup(const BuffGroup *group);

	void setDataLanguage(NamedObject::Language lang);

	const BuffGroup *buffGroup(const QModelIndex &index) const;
	QModelIndex buffGroupIndex(const BuffGroup *group) const;

private:
	QList<const BuffGroup *> listData;
	QMap<const BuffGroup *, int> listDataMap;
	NamedObject::Language dataLanguage;
};

#endif
