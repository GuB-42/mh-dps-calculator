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
	explicit MakeBuilds(bool ia) : ignoreAugmentations(ia) {};
	void operator()(Computer::BuildFutureElt &elt) {
		QVector<Build *> augmented_builds;
		foreach(Build *b, elt.builds) {
			augmented_builds << b;
			if (!ignoreAugmentations) {
				b->fillWeaponAugmentations(&augmented_builds, elt.useful_items);
			}
		}
		elt.builds.clear();
		foreach(Build *b, augmented_builds) {
			elt.builds << b;
			b->fillSlots(&elt.builds, elt.useful_items);
		}
	}
	bool ignoreAugmentations;
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
		if (weapon->type != params.profile->type) continue;
		if (!weapon->final && params.finalOnly) continue;

		BuildFutureElt elt;
		Build *build = new Build;
		build->addWeapon(weapon);
		if (params.ignoreWeaponSlots) {
			build->decorationSlots.clear();
		}
		build->buffLevels = params.buffLevels;
		build->decorationSlots << params.decorationSlots;
		elt.useful_items = build->listUsefulItems(params.items);
		if (params.ignoreWeaponSlots) {
			QVector<Item *>::iterator it = elt.useful_items.begin();
			while (it != elt.useful_items.end()) {
				if ((*it)->buffRefs.isEmpty()) {
					it = elt.useful_items.erase(it);
				} else {
					++it;
				}
			}
		}
		elt.builds.append(build);
		new_future.data.append(elt);
	}

	new_future.future =
		QtConcurrent::map(new_future.data,
		                  MakeBuilds(params.ignoreAugmentations));
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
		bwd->compute(profile, target);
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
		int offset = (max - min) / 5;
		emit progress(min, max + offset, value + offset);
	}
}