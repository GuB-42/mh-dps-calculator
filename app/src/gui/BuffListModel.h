#include <QAbstractListModel>
#include <QList>
#include "../NamedObject.h"

struct Build;
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

	void setDataLanguage(NamedObject::Language lang);

	QMap<const BuffGroup *, int> getBuffLevels() const;
	const BuffGroup *buffGroup(const QModelIndex &index) const;
	QModelIndex buffGroupIndex(const BuffGroup *group) const;

private:
	struct BuffListModelData {
		BuffListModelData() : group(NULL), level(0) {}
		BuffListModelData(const BuffGroup *g, int l) : group(g), level(l) {}
		const BuffGroup *group;
		int level;
	};

	void sortData();

	QList<BuffListModelData> listData;
	QMap<const BuffGroup *, int> listDataMap;
	NamedObject::Language dataLanguage;
};
