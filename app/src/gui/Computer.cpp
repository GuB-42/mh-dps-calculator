#include "Computer.h"

#include <QtConcurrentMap>
#include <QFutureWatcher>

#include "ResultTableModel.h"
#include "../MainData.h"
#include "../Weapon.h"
#include "../Profile.h"
#include "../Item.h"
#include "../BuildWithDps.h"

Computer::Computer(QObject *parent) : QObject(parent) {
}

struct MakeBuilds
{
	explicit MakeBuilds(bool ia, QVector<int> us,
	                    bool ls, QVector<int> sl) :
		ignoreAugmentations(ia), usedSlots(us),
		limitSlots(ls), slotLimit(sl) {};
	void operator()(Computer::BuildFutureElt &elt) {
		QVector<Build *> augmented_builds;
		foreach(Build *b, elt.builds) {
			augmented_builds << b;
			if (!ignoreAugmentations) {
				b->fillWeaponAugmentations(&augmented_builds, elt.useful_items);
			}
		}
		QVector<Build *> slotted_builds;
		foreach(Build *b, augmented_builds) {
			bool enough_slots = true;
			foreach(int s, usedSlots) {
				enough_slots = b->useSlot(s);
				if (!enough_slots) break;
			}
			if (enough_slots) {
				bool downgrade = false;
				if (limitSlots) downgrade = b->limitSlots(slotLimit);
				if (downgrade) {
					delete b;
				} else {
					slotted_builds << b;
					b->fillSlots(&slotted_builds, elt.useful_items);
				}
			} else {
				delete b;
			}
		}
		elt.builds.clear();
		foreach(Build *b, slotted_builds) {
			if (b->weaponSlotUpgrade == 0) {
				elt.builds.append(b);
			} else {
				delete b;
			}
		}
	}
	bool ignoreAugmentations;
	QVector<int> usedSlots;
	bool limitSlots;
	QVector<int> slotLimit;
};

void Computer::compute(const Parameters &params) {
	for (int i = 0; i < buildFutures.count(); ++i) {
		buildFutures[i].future.cancel();
	}
	for (int i = 0; i < resultFutures.count(); ++i) {
		resultFutures[i].future.cancel();
	}

	BuildFuture new_future(params.profile, params.target);
	foreach(const Weapon *weapon, params.weapons) {
		if (weapon->type != params.profile->weaponType) continue;
		if (params.finalOnly) {
			bool not_final = false;
			foreach (const QString &cat, weapon->categoryRefIds) {
				if (cat == "upgradable") {
					not_final = true;
					continue;
				}
			}
			if (not_final) continue;
		}

		BuildFutureElt elt;
		Build *build = new Build;
		build->buffLevels = params.buffLevels;
		build->addWeapon(weapon);
		foreach(int s, params.decorationSlots) build->addSlot(s);
		elt.useful_items = build->listUsefulItems(params.items, *params.profile);
		elt.builds.append(build);
		new_future.data.append(elt);
	}

	new_future.future =
		QtConcurrent::map(new_future.data,
		                  MakeBuilds(params.ignoreAugmentations,
		                             params.usedSlots,
		                             params.ignoreWeaponSlots,
		                             params.decorationSlots));
	buildFutures.append(new_future);
	QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
	watcher->setFuture(new_future.future);
	connect(watcher, SIGNAL(finished()), this, SLOT(buildFutureFinished()));
	connect(watcher, SIGNAL(progressValueChanged(int)), this, SLOT(buildFutureProgress(int)));
	connect(watcher, SIGNAL(finished()), watcher, SLOT(deleteLater()));
}

struct MakeResult
{
	MakeResult(const Profile &p, const Target &t) :
		profile(p), target(t) {};
	void operator()(BuildWithDps *bwd) {
		bwd->compute(profile, target, false);
	}
	const Profile &profile;
	const Target &target;
};

void Computer::buildFutureFinished() {
	while (!buildFutures.isEmpty()) {
		BuildFuture &fdata = buildFutures.first();
		if (!fdata.future.isFinished()) break;
		if (buildFutures.count() == 1 && !fdata.future.isCanceled()) {
			ResultFuture new_future;
			foreach(const BuildFutureElt &elt, fdata.data) {
				foreach(Build *b, elt.builds) {
					new_future.result.append(new BuildWithDps(b));
				}
			}
			new_future.result.squeeze();
			new_future.future =
				QtConcurrent::map(new_future.result,
			                      MakeResult(*fdata.profile, *fdata.target));
			resultFutures.append(new_future);
			QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
			watcher->setFuture(new_future.future);
			connect(watcher, SIGNAL(finished()), this, SLOT(resultFutureFinished()));
			connect(watcher, SIGNAL(progressValueChanged(int)), this, SLOT(resultFutureProgress(int)));
			connect(watcher, SIGNAL(finished()), watcher, SLOT(deleteLater()));
		} else {
			foreach(const BuildFutureElt &elt, fdata.data) {
				foreach(Build *b, elt.builds) delete b;
			}
		}
		buildFutures.removeFirst();
	}
}

void Computer::buildFutureProgress(int value) {
	if (!buildFutures.isEmpty()) {
		int min = buildFutures.last().future.progressMinimum();
		int max = buildFutures.last().future.progressMaximum();
		int offset = (max - min) * 5;
		emit progress(min, max + offset, value);
	}
}

void Computer::resultFutureFinished() {
	while (!resultFutures.isEmpty()) {
		ResultFuture &fdata = resultFutures.first();
		if (!fdata.future.isFinished()) break;
		if (resultFutures.count() == 1 && !fdata.future.isCanceled()) {
			emit finished(fdata.result);
		} else {
			foreach(BuildWithDps *bwd, fdata.result) delete bwd;
		}
		resultFutures.removeFirst();
	}
}

void Computer::resultFutureProgress(int value) {
	if (!resultFutures.isEmpty()) {
		int min = resultFutures.last().future.progressMinimum();
		int max = resultFutures.last().future.progressMaximum();
		int offset = qMax((max - min) / 5, 1);
		emit progress(min, max + offset, value + offset);
	}
}
