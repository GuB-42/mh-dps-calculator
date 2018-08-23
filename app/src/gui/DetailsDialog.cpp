#include "DetailsDialog.h"
#include "ui_DetailsDialog.h"

#include "GuiElements.h"
#include "../Build.h"
#include "../BuildWithDps.h"
#include "../Weapon.h"
#include "../Item.h"
#include "../BuffGroup.h"
#include "../DamageData.h"
#include "../BuffWithCondition.h"

DetailsDialog::DetailsDialog(const Profile *profile,
                             const Target *target,
                             QVector<BuildWithDps *> rd,
                             Language lang,
                             QWidget *parent) :
	QDialog(parent), ui(new Ui::DetailsDialog),
	dataLanguage(lang)
{
	ui->setupUi(this);
	resultData.reserve(rd.count());
	foreach(BuildWithDps *bwd, rd) {
		BuildWithDps *n = new BuildWithDps(*bwd);
		n->compute(*profile, *target, true);
		resultData.append(n);
	}
	updateContent();
}

DetailsDialog::~DetailsDialog() {
	delete ui;
	foreach(BuildWithDps *bwd, resultData) delete bwd;
}

void DetailsDialog::setDataLanguage(Language lang) {
	dataLanguage = lang;
	updateContent();
}

void DetailsDialog::changeEvent(QEvent *event) {
	if (event) {
		switch (event->type()) {
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;
		default:
			break;
		}
	}
	QDialog::changeEvent(event);
}

static QString esc(QString v) {
#if QT_VERSION >= 0x050000
	return v.toHtmlEscaped();
#else
	return Qt::escape(v);
#endif
}

QString DetailsDialog::weaponTableHtml(const Weapon &weapon) {
	QString ret;
	ret += "<table border=\"1\">\n";
	ret += QString("<tr><th>%1</th><th>%2</th></tr>\n").
		arg(tr("Parameter")).arg(tr("Value"));
	ret += QString("<tr><th>%1</th><td align=\"right\">%2</td></tr>\n").
		arg(tr("Attack")).arg(weapon.attack);
	ret += QString("<tr><th>%1</th><td align=\"right\">%2</td></tr>\n").
		arg(tr("Affinity")).arg(weapon.affinity);
	for (int i = 0; i < ELEMENT_COUNT; ++i) {
		if (weapon.elements[i] != 0.0) {
			QString elt = GuiElements::elementName((ElementType)i);
			if (weapon.awakened) elt = tr("(%1)").arg(elt);
			ret += QString("<tr><th>%1</th><td align=\"right\">%2</td></tr>\n").
				arg(esc(elt)).arg(weapon.elements[i]);
		}
		if (weapon.phialElements[i] != 0.0) {
			QString elt =
				tr("%1 phial").arg(GuiElements::elementName((ElementType)i));
			ret += QString("<tr><th>%1</th><td align=\"right\">%2</td></tr>\n").
				arg(esc(elt)).arg(weapon.phialElements[i]);
		}
	}
	for (int i = 0; i < STATUS_COUNT; ++i) {
		if (weapon.statuses[i] != 0.0) {
			QString sta = GuiElements::statusName((StatusType)i);
			if (weapon.awakened) sta = tr("(%1)").arg(sta);
			ret += QString("<tr><th>%1</th><td align=\"right\">%2</td></tr>\n").
				arg(esc(sta)).arg(weapon.statuses[i]);
		}
		if (weapon.phialStatuses[i] != 0.0) {
			QString sta =
				tr("%1 phial").arg(GuiElements::statusName((StatusType)i));
			ret += QString("<tr><th>%1</th><td align=\"right\">%2</td></tr>\n").
				arg(esc(sta)).arg(weapon.phialStatuses[i]);
		}
	}
	ret += "</table>\n";
	return ret;
}

QString DetailsDialog::buffLineHtml(const BuffWithCondition &bwc) {
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
	return QString("<tr><td>%1</td>"
	               "<td align=\"right\">%2</td>"
	               "<td>%3</td></tr>\n").
		arg(esc(buff_name)).
		arg(bwc.value).
		arg(GuiElements::conditionName(bwc.condition));
}

QString DetailsDialog::buffTableHtml(const QVector<const BuffWithCondition *> &bwcs) {
	QString ret;
	ret += "<table border=\"1\">\n";
	ret += QString("<tr><th>%1</th><th>%2</th><th>%3</th></tr>\n").
		arg(tr("Buff")).arg(tr("Value")).arg(tr("Condition"));
	BuffWithCondition tbwc;
	bool first = true;
	foreach(const BuffWithCondition *bwc, bwcs) {
		if (first) {
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
					ret += buffLineHtml(tbwc);
					tbwc = *bwc;
				}
			} else {
				ret += buffLineHtml(tbwc);
				tbwc = *bwc;
			}
		}
	}
	if (!first) ret += buffLineHtml(tbwc);
	ret += "</table>\n";
	return ret;
}

double damage_data_val(const DamageData &dd, int row) {
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

QString damage_data_name(int row) {
	switch (row) {
	case 0: return DetailsDialog::tr("Cutting");
	case 1: return DetailsDialog::tr("Impact");
	case 2: return DetailsDialog::tr("Piercing");
	case 3: return DetailsDialog::tr("Bullet");
	case 4: return DetailsDialog::tr("Fixed");
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

int damage_column_count(const Damage &damage) {
	int ret = 0;
	for (int mode = 0; mode < MODE_COUNT; ++mode) {
		if (!damage.isAlias[mode]) ++ret;
	}
	return ret;
}

QString th_span(QString content, int span) {
	if (span > 1) {
		return QString("<th colspan=\"%1\">%2</th>").arg(span).arg(content);
	} else if (span > 0) {
		return QString("<th>%1</th>").arg(content);
	} else {
		return QString();
	}
}

QString DetailsDialog::damageTableHeaderHtml(const Damage &damage,
                                             QString header1,
                                             QString header2) {
	QString ret;
	int col_count = 0;
	for (int mode = 0; mode < MODE_COUNT; ++mode) {
		if (!damage.isAlias[mode]) ++col_count;
	}
	int row_count = 1;
	if (damage.data[MODE_NORMAL_NORMAL] !=
	    damage.data[MODE_ENRAGED_NORMAL]) ++row_count;
	if (damage.data[MODE_NORMAL_NORMAL] !=
	    damage.data[MODE_NORMAL_WEAK_SPOT]) ++row_count;
	if (!header1.isNull() || !header2.isNull()) {
		if (row_count > 1) {
			ret += QString("<tr><th rowspan=\"%1\">%2</th>%3</tr>\n").
				arg(row_count).arg(header1).arg(th_span(header2, col_count));
		} else {
			ret += QString("<tr><th>%1</th>%2</tr>\n").
				arg(header1).arg(th_span(header2, col_count));
		}
	}
	if (damage.data[MODE_NORMAL_NORMAL] !=
	    damage.data[MODE_ENRAGED_NORMAL]) {
		ret += "<tr>\n";
		QString last_html;
		QString last_title;
		int last_count = 0;
		for (int mode = 0; mode < MODE_COUNT; ++mode) {
			if (damage.isAlias[mode]) continue;
			QString title = (mode == MODE_ENRAGED_NORMAL ||
			                 mode == MODE_ENRAGED_WEAK_SPOT) ?
				tr("Enraged") : tr("Normal");
			if (last_count > 0) {
				if (title != last_title) {
					ret += th_span(last_title, last_count);
					last_title = title;
					last_count = 1;
				} else {
					++last_count;
				}
			} else {
				last_title = title;
				last_count = 1;
			}
		}
		if (last_count > 0) ret += th_span(last_title, last_count);
		ret += "</tr>\n";
	}
	if (damage.data[MODE_NORMAL_NORMAL] !=
	    damage.data[MODE_NORMAL_WEAK_SPOT]) {
		ret += "<tr>\n";
		QString last_html;
		QString last_title;
		int last_count = 0;
		for (int mode = 0; mode < MODE_COUNT; ++mode) {
			if (damage.isAlias[mode]) continue;
			QString title = (mode == MODE_NORMAL_WEAK_SPOT ||
			                 mode == MODE_ENRAGED_WEAK_SPOT) ?
				tr("Weak spot") : tr("Normal");
			if (last_count > 0) {
				if (title != last_title) {
					ret += th_span(last_title, last_count);
					last_title = title;
					last_count = 1;
				} else {
					++last_count;
				}
			} else {
				last_title = title;
				last_count = 1;
			}
		}
		if (last_count > 0) ret += th_span(last_title, last_count);
		ret += "</tr>\n";
	}
	return ret;
}

QString DetailsDialog::damageTableHtml(const Damage &damage) {
	QString ret;
	ret += "<table border=\"1\">\n";
	ret += damageTableHeaderHtml(damage,
	                             tr("Base damage type"),
	                             tr("DPS"));
	for (int row = 0; row < ELEMENT_COUNT + STATUS_COUNT + 5; ++row) {
		bool has_value = false;;
		for (int mode = 0; mode < MODE_COUNT; ++mode) {
			if (damage.isAlias[mode]) continue;
			if (damage_data_val(*damage.data[mode], row) != 0.0) {
				has_value = true;
				break;
			}
		}
		if (has_value) {
			ret += "<tr>\n";
			ret += QString("<th>%1</th>").arg(esc(damage_data_name(row)));
			double d0;
			for (int mode = 0; mode < MODE_COUNT; ++mode) {
				if (damage.isAlias[mode]) continue;
				double d = damage_data_val(*damage.data[mode], row);
				if (mode == 0) d0 = d;
				if (d == d0) {
					ret += QString("<td align=\"right\">%1</td>").arg(d);
				} else {
					ret += QString("<td align=\"right\"><b>%1</b></td>").arg(d);
				}
			}
			ret += "</tr>\n";
		}
	}
	ret += "</table>\n";
	return ret;
}

QString DetailsDialog::damageParamsTableHtml(const Damage &damage) {
	QString ret;
	double d0;
	ret += "<table border=\"1\">\n";
	ret += damageTableHeaderHtml(damage,
	                             tr("Property"),
	                             tr("Value"));
	ret += "<tr>\n";
	ret += QString("<th>%1</th>").arg(tr("Sharpness used"));
	for (int mode = 0; mode < MODE_COUNT; ++mode) {
		if (damage.isAlias[mode]) continue;
		double d = damage.sharpenPeriod * damage.sharpnessUse[mode];
		if (mode == 0) d0 = d;
		if (d0 == d) {
			ret += QString("<td align=\"right\">%1</td>").arg(d);
		} else {
			ret += QString("<td align=\"right\"><b>%1</b></td>").arg(d);
		}
	}
	ret += QString("</tr><tr><th>%1</th>").arg(tr("Mind's eye rate"));
	for (int mode = 0; mode < MODE_COUNT; ++mode) {
		if (damage.isAlias[mode]) continue;
		double d = damage.data[mode]->mindsEyeRate /
			damage.data[mode]->totalRate;
		if (mode == 0) d0 = d;
		if (d0 == d) {
			ret += QString("<td align=\"right\">%1</td>").arg(d);
		} else {
			ret += QString("<td align=\"right\"><b>%1</b></td>").arg(d);
		}
	}
	ret += QString("</tr><tr><th>%1</th>").arg(tr("Sharpness multipliers"));
	for (int mode = 0; mode < MODE_COUNT; ++mode) {
		if (damage.isAlias[mode]) continue;
		ret += QString("<td><table border=\"1\">"
		               "<tr><th>%1</th><th>%2</th></tr>\n").
			arg(tr("Multiplier")).arg(tr("Rate"));
		bool diff_bs =
			damage.data[0]->bounceSharpness.size() !=
			damage.data[mode]->bounceSharpness.size();
		for (size_t i = 0; i < damage.data[mode]->bounceSharpness.size(); ++i) {
			double m0 = 0.0;
			double r0 = 0.0;
			if (!diff_bs) {
				m0 = damage.data[0]->bounceSharpness[i].multiplier;
				r0 = damage.data[0]->bounceSharpness[i].rate /
					damage.data[mode]->totalRate;;
			}
			double m = damage.data[mode]->bounceSharpness[i].multiplier;
			if (m0 == m) {
				ret += QString("<tr><td align=\"right\">%1</td>").arg(m);
			} else {
				ret += QString("<tr><td align=\"right\"><b>%1</b></td>").arg(m);
			}
			double r = damage.data[mode]->bounceSharpness[i].rate /
				damage.data[mode]->totalRate;
			if (r0 == r) {
				ret += QString("<td align=\"right\">%1</td></tr>").arg(r);
			} else {
				ret += QString("<td align=\"right\"><b>%1</b></td></tr>").arg(r);
			}
		}
		ret += "</table></td>\n";
	}
	ret += "</tr></table>\n";
	return ret;
}

double folded_buff_data_val(const FoldedBuffsData &buff, int row) {
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

QString folded_buff_data_name(int row) {
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

QString DetailsDialog::foldedBuffsTableHtml(const Damage &damage) {
	const int nb_rows = NORMAL_BUFF_COUNT +
		ELEMENT_BUFF_COUNT * ELEMENT_COUNT +
		STATUS_BUFF_COUNT * STATUS_COUNT;
	const FoldedBuffsData buff_defaults;

	QString ret;
	ret += "<table border=\"1\">\n";
	ret += damageTableHeaderHtml(damage, tr("Buff"), tr("Effective value"));
	for (int row = 0; row < nb_rows; ++row) {
		bool show_value = false;
		for (int mode = 0; mode < MODE_COUNT; ++mode) {
			if (damage.isAlias[mode]) continue;
			if (!damage.data[mode]->buffData) continue;
			double def = folded_buff_data_val(buff_defaults, row);
			double val = folded_buff_data_val(*damage.data[mode]->buffData, row) /
				 damage.data[mode]->totalRate;
			if (def - val > 1e-9 || val - def > 1e-9) {
				show_value = true;
				break;
			}
		}
		if (show_value) {
			ret += "<tr>\n";
			ret += QString("<th>%1</th>").arg(folded_buff_data_name(row));
			double d0;
			for (int mode = 0; mode < MODE_COUNT; ++mode) {
				if (damage.isAlias[mode]) continue;
				if (!damage.data[mode]->buffData) {
					ret += QString("<td></td>");
					continue;
				}
				double d = folded_buff_data_val(*damage.data[mode]->buffData, row) /
					damage.data[mode]->totalRate;
				if (mode == 0) d0 = d;
				if (d0 == d) {
					ret += QString("<td align=\"right\">%1</td>").arg(d);
				} else {
					ret += QString("<td align=\"right\"><b>%1</b></td>").arg(d);
				}
			}
		}
	}
	ret += "</table>\n";
	return ret;
}

double dps_val(const Dps &dps, int row) {
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


QString dps_name(int row) {
	switch (row) {
	case 0: return DetailsDialog::tr("Total");
	case 1: return DetailsDialog::tr("Raw");
	case 2: return DetailsDialog::tr("Fixed");
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

QString DetailsDialog::dpsTableHtml(const Dps &dps) {
	QString ret;
	ret += "<table border=\"1\">\n";
	ret += QString("<tr><th>%1</th><th>%2</th>\n").
		arg(tr("Damage type")).arg(tr("DPS"));
	for (int row = 0; row < ELEMENT_COUNT + STATUS_COUNT + 5; ++row) {
		if (dps_val(dps, row) != 0.0) {
			ret += QString("<tr><th>%1</th>"
			               "<td align=\"right\">%2</td></tr>\n").
				arg(esc(dps_name(row))).arg(dps_val(dps, row));
		}
	}
	ret += "</table>\n";
	return ret;
}

QString DetailsDialog::statusProcTableHtml(const Dps &dps) {
	QString ret;
	ret += "<table border=\"1\">\n";
	ret += QString("<tr><th>%1</th><th>%2</th><th>%3</th>\n").
		arg(tr("Status")).arg(tr("Rate")).arg(tr("Seconds"));
	for (int i = 0; i < STATUS_COUNT; ++i) {
		if (dps.statusProcRate[i] != 0.0) {
			ret += QString("<tr><th>%1</th>"
			               "<td align=\"right\">%2</td>"
			               "<td align=\"right\">%3</td></tr>\n").
				arg(GuiElements::statusName((StatusType)i)).
				arg(dps.statusProcRate[i]).
				arg(1.0 / dps.statusProcRate[i]);
		}
	}
	ret += "</table>\n";
	return ret;
}

QString DetailsDialog::dpsParamsTableHtml(const Dps &dps) {
	QString ret;
	ret += "<table border=\"1\">\n";
	ret += QString("<tr><th>%1</th><th>%2</th>\n").
		arg(tr("Property")).arg(tr("Value"));
	ret += QString("<tr><th>%1</th><td align=\"right\">%2</td></tr>\n").
		arg(tr("Bounce rate")).arg(dps.bounceRate);
	ret += QString("<tr><th>%1</th><td align=\"right\">%2</td></tr>\n").
		arg(tr("Critical hit rate")).arg(dps.critRate);
	ret += QString("<tr><th>%1</th><td align=\"right\">%2</td></tr>\n").
		arg(tr("Weak spot ratio")).arg(dps.weakSpotRatio);
	ret += QString("<tr><th>%1</th><td align=\"right\">%2</td></tr>\n").
		arg(tr("Kill time (seconds)")).arg(1.0 / dps.killFrequency);
	ret += "</table>\n";
	return ret;
}

QString DetailsDialog::singleDataToHtml(BuildWithDps *bwd) const {
	QString ret;
	ret += QString("<h1>%1</h1>\n").arg(tr("Build"));

	ret += QString("<h2>%1</h2>\n").arg(tr("Weapon"));
	ret += QString("<h3>%1</h3>\n").arg(tr("Name"));
	ret += QString("<p>%1</p>\n").
		arg(esc(bwd->build->weapon->getName(dataLanguage)));
	ret += QString("<h3>%1</h3>\n").arg(tr("Stats"));
	ret += weaponTableHtml(*bwd->build->weapon);

	ret += QString("<h2>%1</h2>\n").arg(tr("Items"));
	ret += "<ul>\n";
	foreach(const Item *item, bwd->build->usedItems) {
		ret += QString("<li>%1</li>\n").
			arg(esc(item->getName(dataLanguage)));
	}
	ret += "</ul>\n";

	ret += QString("<h2>%1</h2>\n").arg(tr("Skills"));
	ret += "<ul>\n";
	foreach(const BuffWithLevel bwl, bwd->build->buffLevels) {
		QString buff_name;
		if (bwl.level >= 0 && bwl.level < bwl.group->levels.count()) {
			const BuffGroupLevel *group_level = bwl.group->levels[bwl.level];
			if (group_level) buff_name = group_level->getName(dataLanguage);
		}
		if (buff_name.isNull()) {
			buff_name = tr("%1 [%2]").
				arg(bwl.group->getName(dataLanguage)).arg(bwl.level);
		}
		ret += QString("<li>%1</li>\n").arg(esc(buff_name));
	}
	ret += "</ul>\n";

	ret += QString("<h2>%1</h2>\n").arg(tr("Buffs"));
	QVector<const BuffWithCondition *> bwcs;
	bwd->build->getBuffWithConditions(&bwcs);
	ret += buffTableHtml(bwcs);

	ret += QString("<h1>%1</h1>\n").arg(tr("Base values"));
	ret += QString("<h2>%1</h2>\n").arg(tr("Damage"));
	ret += damageTableHtml(bwd->damage);
	ret += QString("<h2>%1</h2>\n").arg(tr("Effective buffs"));
	ret += foldedBuffsTableHtml(bwd->damage);
	ret += QString("<h2>%1</h2>\n").arg(tr("Properties"));
	ret += damageParamsTableHtml(bwd->damage);

	ret += QString("<h1>%1</h1>\n").arg(tr("Values against target"));
	ret += QString("<h2>%1</h2>\n").arg(tr("Damage"));
	ret += dpsTableHtml(bwd->dps);
	ret += QString("<h2>%1</h2>\n").arg(tr("Status proc"));
	ret += statusProcTableHtml(bwd->dps);
	ret += QString("<h2>%1</h2>\n").arg(tr("Properties"));
	ret += dpsParamsTableHtml(bwd->dps);

	return ret;
}

QString DetailsDialog::dataToHtml(QVector<BuildWithDps *> bwds) const {
	QString ret;
	bool first = true;
	foreach(BuildWithDps *bwd, bwds) {
		if (!first) ret += "<hr/>\n";
		ret += singleDataToHtml(bwd);
		first = false;
	}
	return ret;
}

void DetailsDialog::updateContent() {
	ui->textBrowser->setHtml(dataToHtml(resultData));
}
