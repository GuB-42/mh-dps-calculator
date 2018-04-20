#ifndef Computer_h_
#define Computer_h_

#include <QObject>
#include <QFuture>
#include "../Build.h"
#include "../NamedObject.h"

struct Profile;
struct Target;
struct MainData;
struct BuildWithDps;
struct Weapon;

class Computer : public QObject {
	Q_OBJECT
public:
	explicit Computer(QObject *parent = NULL);

	struct Parameters {
		Parameters() :
			profile(NULL), target(NULL),
			ignoreAugmentations(false),
			ignoreWeaponSlots(false),
			finalOnly(false) { }

		const Profile *profile;
		const Target *target;
		QVector<Weapon *> weapons;
		QVector<Item *> items;
		QVector<BuffWithLevel> buffLevels;
		QVector<int> decorationSlots;
		bool ignoreAugmentations;
		bool ignoreWeaponSlots;
		bool finalOnly;
	};

public:
	void compute(const Parameters &params);

signals:
	void progress(int min, int max, int value);
	void finished(const QVector<BuildWithDps *> &data);

private slots:
	void buildFutureFinished();
	void buildFutureProgress(int value);
	void resultFutureFinished();
	void resultFutureProgress(int value);

public:
	struct BuildFutureElt {
		QVector<Build *> builds;
		QVector<Item *> useful_items;
	};
	struct BuildFuture {
		BuildFuture() : profile(NULL), target(NULL) { }
		BuildFuture(const Profile *p, const Target *t) :
			profile(p), target(t) { }
		const Profile *profile;
		const Target *target;
		QVector<BuildFutureElt> data;
		QFuture<void> future;
	};
	struct ResultFuture {
		QVector<BuildWithDps *> result;
		QFuture<void> future;
	};

private:
	QList<BuildFuture> buildFutures;
	QList<ResultFuture> resultFutures;
};

#endif
