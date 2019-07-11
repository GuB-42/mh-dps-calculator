#ifndef GeneticComputer_h_
#define GeneticComputer_h_

#include <QObject>
#include <QFuture>
#include "../Build.h"
#include "../NamedObject.h"

struct Profile;
struct Target;
struct MainData;
struct BuildWithDps;
struct Weapon;

class GeneticComputer : public QObject {
	Q_OBJECT
public:
	explicit GeneticComputer(QObject *parent = NULL);

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
		QVector<int> usedSlots;
		bool ignoreAugmentations;
		bool ignoreWeaponSlots;
		bool finalOnly;
	};

public:
	void compute(const Parameters &params);

signals:
	void progress(int min, int max, int value);
	void finished(const QVector<BuildWithDps *> &data);

};

#endif
