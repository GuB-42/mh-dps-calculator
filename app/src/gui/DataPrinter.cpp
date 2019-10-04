#include "DataPrinter.h"

#include <QTextDocument>
#include "../Build.h"
#include "../BuildWithDps.h"
#include "../Weapon.h"
#include "../Item.h"
#include "../BuffGroup.h"
#include "../DamageData.h"
#include "../BuffWithCondition.h"
#include "../Category.h"
#include "GuiElements.h"

static QString esc(const QString &v) {
#if QT_VERSION >= 0x050000
	return v.toHtmlEscaped();
#else
	return Qt::escape(v);
#endif
}

class DPImpl {
	Q_DECLARE_TR_FUNCTIONS(DataPrinter);
public:
	static QString floatCell(double d, bool bold = false, int rowspan = 1);
	static QString startDataTable(QString h1, QString h2);
	static QString startDataTable(QString h1, QString h2, QString h3);
	static QString startDataTable(QString h1, QString h2, QString h3, QString h4);
	static QString startDamageTable(QString h1, QString h2,
	                                const QList<QStringList> &titles);
	static QString endTable();
	static QString simpleDataRow(QString label, double d);
	static QString simpleDataRow(QString label, double d1, double d2);
	static QString buffConditionDataRow(QString label, QString condition,
	                                    QList<double> data, double total);
	static QString damageDataRow(QString label, const bool (&is_alias)[MODE_COUNT],
	                             const double (&data)[MODE_COUNT]);
	static QString notApplicable();
	static QString bounceSharpnessCell(QString h1, QString h2,
	                                   const QList<QPair<double, double> > &data,
	                                   const QList<QPair<double, double> > &base);
	static QString bounceSharpnessRow(QString label, QString h1, QString h2,
	                                  const bool (&is_alias)[MODE_COUNT],
	                                  QList<QPair<double, double> > (&data)[MODE_COUNT]);

	static QList<QStringList> damageTitles(const Damage &damage);
	static bool isDefaultData(const double (&data)[MODE_COUNT], double def);

	static QString weaponTableHtml(const Weapon &weapon);
	static QString buffLineHtml(const BuffWithCondition &bwc,
	                            QList<double> subvals);
	static QString buffTableHtml(const QVector<const BuffWithCondition *> &bwcs);
	static double damageDataVal(const DamageData &dd, int row);
	static QString damageDataName(int row);
	static QString damageTableHtml(const Damage &damage);
	static QString damageParamsTableHtml(const Damage &damage);
	static double foldedBuffDataVal(const FoldedBuffsData &buff, int row);
	static QString foldedBuffDataName(int row);
	static QString foldedBuffsTableHtml(const Damage &damage);
	static double dpsVal(const Dps &dps, int row);
	static QString dpsName(int row);
	static QString dpsTableHtml(const Dps &dps);
	static QString statusProcTableHtml(const Dps &dps);
	static QString dpsParamsTableHtml(const Dps &dps);
	static QString singleDataToHtml(BuildWithDps *bwd, Language lang);
	static QString dataToHtml(const QVector<BuildWithDps *> &bwds,
	                          Language lang);
	static QString htmlHeader();
	static QString htmlFooter();
};

QString DPImpl::floatCell(double d, bool bold, int rowspan) {
	QString rs;
	if (rowspan != 1) rs = QString(" rowspan=\"%1\"").arg(rowspan);
	if (bold) {
		return QString("<td align=\"right\"%1><b>%2</b></td>").arg(rs).arg(d);
	} else {
		return QString("<td align=\"right\"%1>%2</td>").arg(rs).arg(d);
	}
}
QString DPImpl::startDataTable(QString h1, QString h2) {
	return QString("<table border =\"1\" cellspacing=\"1\" cellpadding=\"2\">"
	               "<thead><tr><th>%1</th><th>%2</th></tr></thead>\n").
		arg(esc(h1)).arg(esc(h2));
}
QString DPImpl::startDataTable(QString h1, QString h2, QString h3) {
	return QString("<table border =\"1\" cellspacing=\"1\" cellpadding=\"2\">"
	               "<thead><tr><th>%1</th><th>%2</th><th>%3</th></tr></thead>\n").
		arg(esc(h1)).arg(esc(h2)).arg(esc(h3));
}
QString DPImpl::startDataTable(QString h1, QString h2, QString h3, QString h4) {
	return QString("<table border =\"1\" cellspacing=\"1\" cellpadding=\"2\">"
	               "<thead><tr><th>%1</th><th>%2</th>"
	               "<th>%3</th><th>%4</th></tr></thead>\n").
		arg(esc(h1)).arg(esc(h2)).arg(esc(h3)).arg(esc(h4));
}
QString DPImpl::startDamageTable(QString h1, QString h2,
                                 const QList<QStringList> &titles) {
	if (titles.isEmpty()) return startDataTable(h1, h2);

	QString ret(QString("<table border =\"1\" cellspacing=\"1\" cellpadding=\"2\">"
	                    "<thead><tr><th rowspan=\"%1\">%2</th>"
	                    "<th colspan=\"%3\">%4</th></tr>\n").
		arg(titles.count() + 1).arg(esc(h1)).
		arg(titles.last().count()).arg(esc(h2)));
	foreach(const QStringList &li, titles) {
		int count = 1;
		ret += "<tr>";
		for (int i = 0; i < li.count(); ++i) {
			if (i + 1 < li.count() && li[i] == li[i + 1]) {
				++count;
			} else {
				if (count == 1) {
					ret += QString("<th>%1</th>").arg(esc(li[i]));
				} else {
					ret += QString("<th colspan=\"%1\">%2</th>").
						arg(count).arg(esc(li[i]));
				}
				count = 1;
			}
		}
		ret += "</tr>\n";
	}
	ret += "</thead>\n";
	return ret;
}
QString DPImpl::endTable() {
	return QString("</table>\n");
}
QString DPImpl::simpleDataRow(QString label, double d) {
	return QString("<tr><th>%1</th>%2</tr>\n").
		arg(esc(label)).arg(floatCell(d));
}
QString DPImpl::simpleDataRow(QString label, double d1, double d2) {
	return QString("<tr><th>%1</th>%2%3</tr>\n").
		arg(esc(label)).arg(floatCell(d1)).arg(floatCell(d2));
}
QString DPImpl::buffConditionDataRow(QString label, QString condition,
                                     QList<double> data, double total) {
	QString ret;
	bool first = true;
	foreach(double d, data) {
		if (first) {
			if (data.count() == 1) {
				ret += QString("<tr><th>%1</th>%2%3<td>%4</td></tr>\n").
					arg(esc(label)).arg(floatCell(d)).
					arg(floatCell(total)).arg(esc(condition));
			} else {
				ret += QString("<tr><th rowspan=\"%1\">%2</th>%3%4"
				               "<td rowspan=\"%1\">%5</td></tr>\n").
					arg(data.count()).arg(esc(label)).
					arg(floatCell(d)).
					arg(floatCell(total, false, data.count())).
					arg(esc(condition));
			}
			first = false;
		} else {
			ret += QString("<tr>%1</tr>\n").arg(floatCell(d));
		}
	}
	return ret;
}
QString DPImpl::damageDataRow(QString label, const bool (&is_alias)[MODE_COUNT],
                              const double (&data)[MODE_COUNT]) {
	QString ret(QString("<tr><th>%1</th>").arg(esc(label)));
	for (int i = 0; i < MODE_COUNT; ++i) {
		if (!is_alias[i]) ret += floatCell(data[i], data[i] != data[0]);
	}
	ret += "</tr>\n";
	return ret;
}
QString DPImpl::notApplicable() {
	return "<p>N/A</p>\n";
}

QString DPImpl::bounceSharpnessCell(QString h1, QString h2,
	                                const QList<QPair<double, double> > &data,
	                                const QList<QPair<double, double> > &base) {
	QString ret("<td>");
	if (!data.isEmpty()) {
		ret += startDataTable(h1, h2);
		for (int i = 0; i < data.size(); ++i) {
			bool has_base = (i < base.count());
			ret += "<tr>";
			ret += floatCell(data[i].first,
			                 has_base && base[i].first != data[i].first);
			ret += floatCell(data[i].second,
			                 has_base && base[i].second != data[i].second);
			ret += "</tr>\n";
		}
		ret += endTable();
	}
	ret += "</td>\n";
	return ret;
}
QString DPImpl::bounceSharpnessRow(QString label, QString h1, QString h2,
                                   const bool (&is_alias)[MODE_COUNT],
                                   QList<QPair<double, double> > (&data)[MODE_COUNT]) {
	QString ret(QString("<tr><th>%1</th>").arg(esc(label)));
	for (int i = 0; i < MODE_COUNT; ++i) {
		if (!is_alias[i]) ret += bounceSharpnessCell(h1, h2, data[i], data[0]);
	}
	ret += "</tr>\n";
	return ret;
}

QList<QStringList> DPImpl::damageTitles(const Damage &damage) {
	QList<QStringList> ret;
	if (!damage.isAlias[MODE_ENRAGED_NORMAL]) {
		ret.append(QStringList());
		for (int i = 0; i < MODE_COUNT; ++i) {
			if (damage.isAlias[i]) continue;
			ret.last().append(i == MODE_ENRAGED_NORMAL ||
			                  i == MODE_ENRAGED_WEAK_SPOT ?
			                  tr("Enraged") : tr("Normal"));
		}
	}
	if (!damage.isAlias[MODE_NORMAL_WEAK_SPOT]) {
		ret.append(QStringList());
		for (int i = 0; i < MODE_COUNT; ++i) {
			if (damage.isAlias[i]) continue;
			ret.last().append(i == MODE_NORMAL_WEAK_SPOT ||
			                  i == MODE_ENRAGED_WEAK_SPOT ?
			                  tr("Weak spot") : tr("Normal"));
		}
	}
	return ret;
}

bool DPImpl::isDefaultData(const double (&data)[MODE_COUNT], double def) {
	for (int i = 0; i < MODE_COUNT; ++i) {
		if (data[i] - def > 1e-9 || def - data[i] > 1e-9) return false;
	}
	return true;
}

QString DPImpl::weaponTableHtml(const Weapon &weapon) {
	QString ret;
	ret += startDataTable(tr("Parameter"), tr("Value"));
	ret += simpleDataRow(tr("Attack"), weapon.attack);
	ret += simpleDataRow(tr("Affinity"), weapon.affinity);
	for (int i = 0; i < ELEMENT_COUNT; ++i) {
		if (weapon.elements[i] != 0.0) {
			QString elt = GuiElements::elementName((ElementType)i);
			if (weapon.awakened) elt = tr("(%1)").arg(elt);
			ret += simpleDataRow(elt, weapon.elements[i]);
		}
		if (weapon.phialElements[i] != 0.0) {
			QString elt = tr("%1 phial").
				arg(GuiElements::elementName((ElementType)i));
			ret += simpleDataRow(elt, weapon.phialElements[i]);
		}
	}
	for (int i = 0; i < STATUS_COUNT; ++i) {
		if (weapon.statuses[i] != 0.0) {
			QString sta = GuiElements::statusName((StatusType)i);
			if (weapon.awakened) sta = tr("(%1)").arg(sta);
			ret += simpleDataRow(sta, weapon.statuses[i]);
		}
		if (weapon.phialStatuses[i] != 0.0) {
			QString sta = tr("%1 phial").
				arg(GuiElements::statusName((StatusType)i));
			ret += simpleDataRow(sta, weapon.phialStatuses[i]);
		}
	}
	ret += endTable();
	return ret;
}

QString DPImpl::buffLineHtml(const BuffWithCondition &bwc,
                             QList<double> subvals) {
	QString buff_name;
	switch (bwc.buffClass) {
	case BuffWithCondition::BUFF_CLASS_NONE:
		break;
	case BuffWithCondition::BUFF_CLASS_NORMAL:
		buff_name = GuiElements::normalBuffName(bwc.normal.buff);
		break;
	case BuffWithCondition::BUFF_CLASS_ELEMENT:
		buff_name = GuiElements::elementBuffName(bwc.element.buff,
		                                         bwc.element.type);
		break;
	case BuffWithCondition::BUFF_CLASS_STATUS:
		buff_name = GuiElements::statusBuffName(bwc.status.buff,
		                                        bwc.status.type);
		break;
	}
	return buffConditionDataRow(buff_name,
	                            GuiElements::conditionName(bwc.condition),
	                            subvals, bwc.value);
}

QString DPImpl::buffTableHtml(const QVector<const BuffWithCondition *> &bwcs) {
	QString ret;
	BuffWithCondition tbwc;
	bool first = true;
	QList<double> subvals;
	foreach(const BuffWithCondition *bwc, bwcs) {
		if (first) {
			ret += startDataTable(tr("Buff"), tr("Value"),
			                      tr("Total"), tr("Condition"));
			tbwc = *bwc;
			first = false;
		} else {
			if (tbwc.sameBuffAs(*bwc)) {
				if (tbwc.condition == bwc->condition) {
					switch (tbwc.combineOp()) {
					case BuffWithCondition::OP_NONE:
						break;
					case BuffWithCondition::OP_AFFINITY:
						tbwc.value += bwc->value;
						break;
					case BuffWithCondition::OP_PLUS:
						tbwc.value += bwc->value;
						break;
					case BuffWithCondition::OP_MULTIPLY:
						tbwc.value *= bwc->value;
						break;
					case BuffWithCondition::OP_MAX:
						if (bwc->value > tbwc.value) tbwc.value = bwc->value;
						break;
					}
				} else {
					ret += buffLineHtml(tbwc, subvals);
					tbwc = *bwc;
					subvals.clear();
				}
			} else {
				ret += buffLineHtml(tbwc, subvals);
				tbwc = *bwc;
				subvals.clear();
			}
		}
		subvals.append(bwc->value);
	}
	if (first) {
		ret += notApplicable();
	} else {
		ret += buffLineHtml(tbwc, subvals);
		ret += endTable();
	}
	return ret;
}

double DPImpl::damageDataVal(const DamageData &dd, int row) {
	switch (row) {
	case 0: return dd.cut;
	case 1: return dd.impact;
	case 2: return dd.piercing;
	case 3: return dd.bullet;
	case 4: return dd.fixed;
	}
	if (row >= 5 && row < 5 + ELEMENT_COUNT) {
		return dd.elements[row - 5];
	}
	if (row >= 5 + ELEMENT_COUNT &&
	    row < 5 + ELEMENT_COUNT + STATUS_COUNT) {
		return dd.statuses[row - 5 - ELEMENT_COUNT];
	}
	return 0.0;
}

QString DPImpl::damageDataName(int row) {
	switch (row) {
	case 0: return tr("Cutting");
	case 1: return tr("Impact");
	case 2: return tr("Piercing");
	case 3: return tr("Bullet");
	case 4: return tr("Fixed");
	}
	if (row >= 5 && row < 5 + ELEMENT_COUNT) {
		return GuiElements::elementName(ElementType(row - 5));
	}
	if (row >= 5 + ELEMENT_COUNT &&
	    row < 5 + ELEMENT_COUNT + STATUS_COUNT) {
		return GuiElements::statusName(StatusType(row - 5 - ELEMENT_COUNT));
	}
	return QString();
}

QString DPImpl::damageTableHtml(const Damage &damage) {
	QString ret;
	bool found = false;
	for (int row = 0; row < ELEMENT_COUNT + STATUS_COUNT + 5; ++row) {
		double dt[MODE_COUNT];
		for (int i = 0; i < MODE_COUNT; ++i) {
			dt[i] = damageDataVal(*damage.data[i], row);
		}
		if (!isDefaultData(dt, 0.0)) {
			if (!found) {
				ret += startDamageTable(tr("Base damage type"), tr("DPS"),
				                        damageTitles(damage));
				found = true;
			}
			ret += damageDataRow(damageDataName(row), damage.isAlias, dt);
		}
	}
	ret += found ? endTable() : notApplicable();
	return ret;
}

QString DPImpl::damageParamsTableHtml(const Damage &damage) {
	QString ret;
	ret += startDamageTable(tr("Property"), tr("Value"),
	                        damageTitles(damage));
	double dt[MODE_COUNT];
	for (int i = 0; i < MODE_COUNT; ++i) {
		dt[i] = damage.sharpenPeriod * damage.sharpnessUse[i];
	}
	ret += damageDataRow(tr("Sharpness used"), damage.isAlias, dt);
	for (int i = 0; i < MODE_COUNT; ++i) {
		dt[i] = damage.data[i]->mindsEyeRate / damage.data[i]->totalRate;
	}
	ret += damageDataRow(tr("Mind's eye rate"), damage.isAlias, dt);

	QList<QPair<double, double> > bs_data[MODE_COUNT];
	for (int i = 0; i < MODE_COUNT; ++i) {
		const DamageData::BounceSharpnessArray &bs =
			damage.data[i]->bounceSharpness;
		for (size_t j = 0; j < bs.size(); ++j) {
			double m = damage.data[i]->bounceSharpness[j].multiplier;
			double r = damage.data[i]->bounceSharpness[j].rate /
				damage.data[i]->totalRate;
			bs_data[i].append(QPair<double, double>(m, r));
		}
	}
	ret += bounceSharpnessRow(tr("Sharpness multipliers"),
	                          tr("Multiplier"), tr("Rate"),
	                          damage.isAlias,
	                          bs_data);
	ret += endTable();
	return ret;
}

double DPImpl::foldedBuffDataVal(const FoldedBuffsData &buff, int row) {
	const int status_ofs =
		NORMAL_BUFF_COUNT + ELEMENT_BUFF_COUNT * ELEMENT_COUNT;
	if (row < NORMAL_BUFF_COUNT) {
		return buff.normalBuffs[row];
	} else if (row < status_ofs) {
		int tr = row - NORMAL_BUFF_COUNT;
		return buff.elementBuffs[tr / ELEMENT_COUNT][tr % ELEMENT_COUNT];
	} else if (row < status_ofs + STATUS_BUFF_COUNT * STATUS_COUNT) {
		int tr = row - status_ofs;
		return buff.statusBuffs[tr / STATUS_COUNT][tr % STATUS_COUNT];
	}
	return 0.0;
}

QString DPImpl::foldedBuffDataName(int row) {
	const int status_ofs =
		NORMAL_BUFF_COUNT + ELEMENT_BUFF_COUNT * ELEMENT_COUNT;
	if (row < NORMAL_BUFF_COUNT) {
		return GuiElements::normalBuffName((NormalBuff)row);
	} else if (row < status_ofs) {
		int tr = row - NORMAL_BUFF_COUNT;
		return GuiElements::elementBuffName((ElementBuff)(tr / ELEMENT_COUNT),
		                                    (ElementType)(tr % ELEMENT_COUNT));
	} else if (row < status_ofs + STATUS_BUFF_COUNT * STATUS_COUNT) {
		int tr = row - status_ofs;
		return GuiElements::statusBuffName((StatusBuff)(tr / STATUS_COUNT),
		                                   (StatusType)(tr % STATUS_COUNT));
	}
	return QString();
}

QString DPImpl::foldedBuffsTableHtml(const Damage &damage) {
	const int nb_rows = NORMAL_BUFF_COUNT +
		ELEMENT_BUFF_COUNT * ELEMENT_COUNT +
		STATUS_BUFF_COUNT * STATUS_COUNT;
	const FoldedBuffsData buff_defaults;

	QString ret;
	bool found = false;
	for (int row = 0; row < nb_rows; ++row) {
		double dt[MODE_COUNT];
		for (int i = 0; i < MODE_COUNT; ++i) {
			dt[i] = foldedBuffDataVal(*damage.data[i]->buffData, row) /
				damage.data[i]->totalRate;
		}
		if (!isDefaultData(dt, foldedBuffDataVal(buff_defaults, row))) {
			if (!found) {
				ret += startDamageTable(tr("Buff"), tr("Effective value"),
				                        damageTitles(damage));
				found = true;
			}
			ret += damageDataRow(foldedBuffDataName(row), damage.isAlias, dt);
		}
	}
	ret += found ? endTable() : notApplicable();
	return ret;
}

double DPImpl::dpsVal(const Dps &dps, int row) {
	switch (row) {
	case 0: return dps.totalDps();
	case 1: return dps.raw;
	case 2: return dps.fixed;
	}
	if (row >= 2 && row < 2 + ELEMENT_COUNT) {
		return dps.elements[row - 2];
	}
	if (row >= 2 + ELEMENT_COUNT &&
	    row < 2 + ELEMENT_COUNT + STATUS_COUNT) {
		return dps.statuses[row - 2 - ELEMENT_COUNT];
	}
	return 0.0;
}


QString DPImpl::dpsName(int row) {
	switch (row) {
	case 0: return tr("Total");
	case 1: return tr("Raw");
	case 2: return tr("Fixed");
	}
	if (row >= 2 && row < 2 + ELEMENT_COUNT) {
		return GuiElements::elementName(ElementType(row - 2));
	}
	if (row >= 2 + ELEMENT_COUNT &&
	    row < 2 + ELEMENT_COUNT + STATUS_COUNT) {
		return GuiElements::statusName(StatusType(row - 2 - ELEMENT_COUNT));
	}
	return QString();
}

QString DPImpl::dpsTableHtml(const Dps &dps) {
	QString ret;
	ret += startDataTable(tr("Damage type"), tr("DPS"));
	for (int row = 0; row < ELEMENT_COUNT + STATUS_COUNT + 5; ++row) {
		if (row == 0 || dpsVal(dps, row) != 0.0) {
			ret += simpleDataRow(dpsName(row), dpsVal(dps, row));
		}
	}
	ret += endTable();
	return ret;
}

QString DPImpl::statusProcTableHtml(const Dps &dps) {
	QString ret;
	bool found = false;
	for (int i = 0; i < STATUS_COUNT; ++i) {
		if (dps.statusProcRate[i] != 0.0) {
			if (!found) {
				ret += startDataTable(tr("Status"), tr("Rate"), tr("Seconds"));
				found = true;
			}
			ret += simpleDataRow(GuiElements::statusName((StatusType)i),
			                     dps.statusProcRate[i],
			                     1.0 / dps.statusProcRate[i]);
		}
	}
	ret += found ? endTable() : notApplicable();
	return ret;
}

QString DPImpl::dpsParamsTableHtml(const Dps &dps) {
	QString ret;
	ret += startDataTable(tr("Property"), tr("Value"));
	ret += simpleDataRow(tr("Bounce rate"), dps.bounceRate);
	ret += simpleDataRow(tr("Critical hit rate"), dps.critRate);
	ret += simpleDataRow(tr("Weak spot ratio"), dps.weakSpotRatio);
	ret += simpleDataRow(tr("Kill time (seconds)"), 1.0 / dps.killFrequency);
	ret += endTable();
	return ret;
}

QString DPImpl::singleDataToHtml(BuildWithDps *bwd, Language lang) {
	QString ret;
	ret += QString("<h1>%1</h1>\n").arg(esc(tr("Build")));

	ret += QString("<h2>%1</h2>\n").arg(esc(tr("Weapon")));
	ret += QString("<h3>%1</h3>\n").arg(esc(tr("Name")));
	ret += QString("<p>%1</p>\n").
		arg(esc(bwd->build->weapon->getName(lang)));
	ret += QString("<h3>%1</h3>\n").arg(esc(tr("Categories")));
	if (!bwd->build->weapon->categories.isEmpty()) {
		ret += "<ul>\n";
		foreach(const Category *cat, bwd->build->weapon->categories) {
			ret += QString("<li>%1</li>\n").
				arg(esc(cat->getName(lang)));
		}
		ret += "</ul>\n";
	} else {
		ret += notApplicable();
	}
	ret += QString("<h3>%1</h3>\n").arg(esc(tr("Stats")));
	ret += weaponTableHtml(*bwd->build->weapon);

	ret += QString("<h2>%1</h2>\n").arg(esc(tr("Items")));
	if (!bwd->build->usedItems.isEmpty()) {
		ret += "<ul>\n";
		foreach(const Item *item, bwd->build->usedItems) {
			ret += QString("<li>%1</li>\n").
				arg(esc(item->getName(lang)));
		}
		ret += "</ul>\n";
	} else {
		ret += notApplicable();
	}

	ret += QString("<h2>%1</h2>\n").arg(esc(tr("Skills")));
	if (!bwd->build->buffLevels.isEmpty()) {
		ret += "<ul>\n";
		foreach(const BuffWithLevel bwl, bwd->build->buffLevels) {
			QString buff_name;
			if (bwl.level >= 0 && bwl.level < bwl.group->levels.count()) {
				const BuffGroupLevel *group_level = bwl.group->levels[bwl.level];
				if (group_level) buff_name = group_level->getName(lang);
			}
			if (buff_name.isNull()) {
				buff_name = tr("%1 [%2]").
					arg(bwl.group->getName(lang)).arg(bwl.level);
			}
			ret += QString("<li>%1</li>\n").arg(esc(buff_name));
		}
		ret += "</ul>\n";
	} else {
		ret += notApplicable();
	}

	ret += QString("<h2>%1</h2>\n").arg(esc(tr("Buffs")));
	QVector<const BuffWithCondition *> bwcs;
	bwd->build->getBuffWithConditions(&bwcs);
	ret += buffTableHtml(bwcs);

	ret += QString("<h1>%1</h1>\n").arg(esc(tr("Base values")));
	ret += QString("<h2>%1</h2>\n").arg(esc(tr("Damage")));
	ret += damageTableHtml(bwd->damage);
	ret += QString("<h2>%1</h2>\n").arg(esc(tr("Effective buffs")));
	ret += foldedBuffsTableHtml(bwd->damage);
	ret += QString("<h2>%1</h2>\n").arg(esc(tr("Properties")));
	ret += damageParamsTableHtml(bwd->damage);

	ret += QString("<h1>%1</h1>\n").arg(esc(tr("Values against target")));
	ret += QString("<h2>%1</h2>\n").arg(esc(tr("Damage")));
	ret += dpsTableHtml(bwd->dps);
	ret += QString("<h2>%1</h2>\n").arg(esc(tr("Status proc")));
	ret += statusProcTableHtml(bwd->dps);
	ret += QString("<h2>%1</h2>\n").arg(esc(tr("Properties")));
	ret += dpsParamsTableHtml(bwd->dps);

	return ret;
}

QString DPImpl::dataToHtml(const QVector<BuildWithDps *> &bwds,
                                   Language lang) {
	QString ret;
	bool first = true;
	foreach(BuildWithDps *bwd, bwds) {
		if (!first) ret += "<hr />\n";
		ret += singleDataToHtml(bwd, lang);
		first = false;
	}
	return ret;
}

QString DPImpl::htmlHeader() {
	return QString("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n"
			       " \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"
	               "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\n"
	               "<head>\n"
	               "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\" />\n"
	               "<title>%1</title>\n"
	               "</head>\n"
	               "<body>\n").arg(esc(tr("DPS details")));
}

QString DPImpl::htmlFooter() {
	return QString("</body>\n</html>\n");
}

QString DataPrinter::singleDataToHtml(BuildWithDps *bwd, Language lang) {
	QString ret(DPImpl::htmlHeader());
	ret += DPImpl::singleDataToHtml(bwd, lang);
	ret += DPImpl::htmlFooter();
	return ret;
}

QString DataPrinter::dataToHtml(const QVector<BuildWithDps *> &bwds,
                                Language lang) {
	QString ret(DPImpl::htmlHeader());
	ret += DPImpl::dataToHtml(bwds, lang);
	ret += DPImpl::htmlFooter();
	return ret;
}
