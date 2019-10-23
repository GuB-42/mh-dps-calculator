#!/usr/bin/perl

# http://mhw.wiki-db.com/sim/compiled/sim-compiled.js
# http://mhw.wiki-db.com/sim/compiled/sim-compiled-en.js

use strict;
use warnings;

use utf8;
use open ':std', ':encoding(UTF-8)';

use XML::Writer;
use JSON;
use Data::Dumper;

my $xml_writer;

my %buff_name_id = (
	"Poison Resistance" => "poison_resistance",
	"Paralysis Resistance" => "paralysis_resistance",
	"Sleep Resistance" => "sleep_resistance",
	"Stun Resistance" => "stun_resistance",
	"Muck Resistance" => "muck_resistance",
	"Blast Resistance" => "blast_resistance",
	"Bleeding Resistance" => "bleeding_resistance",
	"Iron Skin" => "iron_skin",
	"Earplugs" => "earplugs",
	"Windproof" => "windproof",
	"Tremor Resistance" => "tremor_resistance",
	"Dungmaster" => "dungmaster",
	"Effluvial Expert" => "effluvial_expert",
	"Heat Guard" => "heat_guard",
	"Attack Boost" => "attack_boost",
	"Defense Boost" => "defense_boost",
	"Health Boost" => "health_boost",
	"Recovery Up" => "recovery_up",
	"Recovery Speed" => "recovery_speed",
	"Fire Resistance" => "fire_resistance",
	"Water Resistance" => "water_resistance",
	"Ice Resistance" => "ice_resistance",
	"Thunder Resistance" => "thunder_resistance",
	"Dragon Resistance" => "dragon_resistance",
	"Blight Resistance" => "blight_resistance",
	"Fire Attack" => "fire_attack",
	"Water Attack" => "water_attack",
	"Ice Attack" => "ice_attack",
	"Thunder Attack" => "thunder_attack",
	"Dragon Attack" => "dragon_attack",
	"Poison Attack" => "poison_attack",
	"Paralysis Attack" => "paralysis_attack",
	"Sleep Attack" => "sleep_attack",
	"Blast Attack" => "blast_attack",
	"Normal Shots" => "normal_shots",
	"Piercing Shots" => "piercing_shots",
	"Spread/Power Shots" => "spread_power_shots",
	"Free Elem/Ammo Up" => "free_elem_ammo_up",
	"Critical Eye" => "critical_eye",
	"Critical Boost" => "critical_boost",
	"Weakness Exploit" => "weakness_exploit",
	"Focus" => "focus",
	"Power Prolonger" => "power_prolonger",
	"Handicraft" => "handicraft",
	"Critical Draw" => "critical_draw",
	"Partbreaker" => "partbreaker",
	"Slugger" => "slugger",
	"Stamina Thief" => "stamina_thief",
	"Master Mounter" => "master_mounter",
	"Airborne" => "airborne",
	"Latent Power" => "latent_power",
	"Agitator" => "agitator",
	"Peak Performance" => "peak_performance",
	"Heroics" => "heroics",
	"Fortify" => "fortify",
	"Resentment" => "resentment",
	"Resuscitate" => "resuscitate",
	"Horn Maestro" => "horn_maestro",
	"Capacity Boost" => "capacity_boost",
	"Special Ammo Boost" => "special_ammo_boost",
	"Artillery" => "artillery",
	"Heavy Artillery" => "heavy_artillery",
	"Marathon Runner" => "marathon_runner",
	"Constitution" => "constitution",
	"Leap of Faith" => "leap_of_faith",
	"Stamina Surge" => "stamina_surge",
	"Hunger Resistance" => "hunger_resistance",
	"Evade Window" => "evade_window",
	"Evade Extender" => "evade_extender",
	"Guard" => "guard",
	"Quick Sheath" => "quick_sheath",
	"Wide-Range" => "wide_range",
	"Item Prolonger" => "item_prolonger",
	"Free Meal" => "free_meal",
	"Speed Eating" => "speed_eating",
	"Speed Sharpening" => "speed_sharpening",
	"Bombardier" => "bombardier",
	"Mushroomancer" => "mushroomancer",
	"Master Fisher" => "master_fisher",
	"Pro Transporter" => "pro_transporter",
	"Master Gatherer" => "master_gatherer",
	"Honey Hunter" => "honey_hunter",
	"Carving Pro" => "carving_pro",
	"Divine Blessing" => "divine_blessing",
	"Palico Rally" => "palico_rally",
	"Botanist" => "botanist",
	"Geologist" => "geologist",
	"Maximum Might" => "maximum_might",
	"Slinger Capacity" => "slinger_capacity",
	"Stealth" => "stealth",
	"Flinch Free" => "flinch_free",
	"Scoutfly Range Up" => "scoutfly_range_up",
	"Speed Crawler" => "speed_crawler",
	"Jump Master" => "jump_master",
	"Sporepuff Expert" => "sporepuff_expert",
	"Aquatic Expert" => "aquatic_expert",
	"Cliffhanger" => "cliffhanger",
	"Blindsider" => "blindsider",
	"Scholar" => "scholar",
	"Entomologist" => "entomologist",
	"Effluvia Resistance" => "effluvia_resistance",
	"Scenthound" => "scenthound",
	"Forager's Luck" => "foragers_luck",
	"Detector" => "detector",
	"BBQ Master" => "bbq_master",
	"Tool Specialist" => "tool_specialist",
	"Affinity Sliding" => "affinity_sliding",
	"Intimidator" => "intimidator",
	"Hasten Recovery" => "hasten_recovery",
	"Super Recovery" => "super_recovery",
	"Poison Duration Up" => "poison_duration_up",
	"Adrenaline" => "adrenaline",
	"Stamina Cap Up" => "stamina_cap_up",
	"Critical Element" => "critical_element",
	"Mind's Eye/Ballistics" => "minds_eye_ballistics",
	"Bludgeoner" => "bludgeoner",
	"Non-elemental Boost" => "non_elemental_boost",
	"Capture Master" => "capture_master",
	"Guard Up" => "guard_up",
	"Guts" => "guts",
	"Master's Touch" => "masters_touch",
	"Nullify Wind Pressure" => "nullify_wind_pressure",
	"Razor Sharp/Spare Shot" => "razor_sharp_spare_shot",
	"Critical Status" => "critical_status",
	"Good Luck" => "good_luck",
	"Bow Charge Plus" => "bow_charge_plus",
	"Punishing Draw" => "punishing_draw",
	"Protective Polish" => "protective_polish",
	"Poison Functionality" => "poison_functionality",
	"Para Functionality" => "para_functionality",
	"Sleep Functionality" => "sleep_functionality",
	"Blast Functionality" => "blast_functionality",
	"Elderseal Boost" => "elderseal_boost",
	"Carving Master" => "carving_master",
	"Great Luck" => "great_luck",
	"Elemental Airborne" => "elemental_airborne",
	"Coalescence" => "coalescence",
	"Offensive Guard" => "offensive_guard",
	"Safe Landing" => "safe_landing",
	"Provoker" => "provoker",
	"Agitator Secret" => "agitator_secret",
	"Artillery Secret" => "artillery_secret",
	"Bombardier Secret" => "bombardier_secret",
	"Frostcraft" => "frostcraft",
	"Divine Blessing Secret" => "divine_blessing_secret",
	"True Critical Status" => "true_critical_status",
	"Element Acceleration" => "element_acceleration",
	"True Element Acceleration" => "true_element_acceleration",
	"Free Meal Secret" => "free_meal_secret",
	"Gaia's Veil" => "gaias_veil",
	"True Gaia's Veil" => "true_gaias_veil",
	"Latent Power Secret" => "latent_power_secret",
	"Maximum Might Secret" => "maximum_might_secret",
	"Slinger Ammo Secret" => "slinger_ammo_secret",
	"True Critical Element" => "true_critical_element",
	"Slugger Secret" => "slugger_secret",
	"Stamina Thief Secret" => "stamina_thief_secret",
	"Tool Specialist Secret" => "tool_specialist_secret",
	"True Razor Sharp/Spare Shot" => "true_razor_sharp_spare_shot",

	"Defence Boost" => "defense_boost",
	"Paralysis Functionality" => "para_functionality",
	"Survival Expert" => "sporepuff_expert",
	"Effluvial Resistance" => "effluvia_resistance",
	"Scent Hound" => "scent_hound",
	"Aquatic/Polar Mobility" => "aquatic_expert",
	"陽動攻撃" => "provoker",
);

my %set_bonus_name_id = (
	"Anjanath Power" => "anjanath_power",
	"Legiana Blessing" => "legiana_blessing",
	"Odogaron Power" => "odogaron_power",
	"Rathalos Power" => "rathalos_power",
	"Diablos Power" => "diablos_power",
	"Kirin Blessing" => "kirin_blessing",
	"Anjanath Will" => "anjanath_will",
	"Diablos Mastery" => "diablos_mastery",
	"Legiana Favor" => "legiana_favor",
	"Kirin Favor" => "kirin_favor",
	"Commission Guidance" => "commission_guidance",
	"Rathalos Mastery" => "rathalos_mastery",
	"Zorah Magdaros Mastery" => "zorah_magdaros_mastery",
	"Guild Guidance" => "guild_guidance",
	"Uragaan Protection" => "uragaan_protection",
	"Bazelgeuse Protection" => "bazelgeuse_protection",
	"Nergigante Hunger" => "nergigante_hunger",
	"Teostra Technique" => "teostra_technique",
	"Kushala Daora Flight" => "kushala_daora_flight",
	"Pink Rathian Mastery" => "pink_rathian_mastery",
	"Odogaron Mastery" => "odogaron_mastery",
	"Xeno'jiiva Divinity" => "xeno_jiiva_divinity",
	"Vaal Hazak Vitality" => "vaal_hazak_vitality",
	"Lunastra Favor" => "lunastra_favor",
	"Astera Blessing" => "astera_blessing",

	"Soul of the Dragoon" => "soul_of_the_dragoon",
	"Brachydios Essence" => "brachydios_essence",
	"Zorah Magdaros Essence" => "zorah_magdaros_essence",
	"Commission Alchemy" => "commission_alchemy",
	"Legiana Ambition" => "legiana_ambition",
	"Instructor's Guidance" => "instructors_guidance",
	"Commission Guidance" => "commission_guidance",
	"Guild Pride" => "guild_pride",
	"Velkhana Divinity" => "velkhana_divinity",
	"Ancient Divinity" => "ancient_divinity",
	"Gold Rathian Essence" => "gold_rathian_essence",
	"Namielle Divinity" => "namielle_divinity",
	"Tigrex Essence" => "tigrex_essence",
	"Shara Ishvalda Divinity" => "shara_ishvalda_divinity",
	"Kirin Divinity" => "kirin_divinity",
	"Uragaan Ambition" => "uragaan_ambition",
	"Bazelgeuse Ambition" => "bazelgeuse_ambition",
	"Nergigante Ambition" => "nergigante_ambition",
	"Zinogre Essence" => "zinogre_essence",
	"Glavenus Essence" => "glavenus_essence",
	"Rathalos Essence" => "rathalos_essence",
	"Rajang's Rage" => "rajangs_rage",
	"Rathian Essence" => "rathian_essence",
	"Odogaron Essence" => "odogaron_essence",
	"Barioth Hidden Art" => "barioth_hidden_art",
	"Silver Rathalos Essence" => "silver_rathalos_essence",
	"Diablos Ambition" => "diablos_ambition",
	"Anjanath Dominance" => "anjanath_dominance",
	"Deviljho Essence" => "deviljho_essence",
	"Vaal Soulvein" => "vaal_soulvein",
	"Lunastra Essence" => "lunastra_essence",
	"Nargacuga Essence" => "nargacuga_essence",

	"Soul of the Dragon" => "soul_of_the_dragoon",
	"Val Hazak Vitality" => "vaal_hazak_vitality",
	"Legian Ambition" => "legiana_ambition",
	"金獅子の怒気" => "rajangs_rage"
);

my %part_name = (
	0 => "head",
	1 => "chest",
	2 => "arms",
	3 => "waist",
	4 => "legs",
	5 => "charm"
);

my %resist_name = (
	0 => "fire",
	1 => "water",
	2 => "thunder",
	3 => "ice",
	4 => "dragon",
);

sub mod_name {
	my $n = $_[0];
	$n =~ s/\s*alpha\s*(\+?)\s*$/ α$1/i;
	$n =~ s/\s*beta\s*(\+?)\s*$/ β$1/i;
	$n =~ s/\s*gamma\s*(\+?)\s*$/ γ$1/i;
	return $n;
}

sub name_to_id {
	my $n = $_[0];
	$n =~ s/[_\W]+/_/g;
	$n =~ s/^\s*|\s*$//;
	return lc($n);
}

my $io = IO::Handle->new();
$io->fdopen(fileno(STDOUT), ">");
$xml_writer = XML::Writer->new(OUTPUT => $io, ENCODING => 'utf-8', DATA_MODE => 1, DATA_INDENT => 4);
$xml_writer->xmlDecl("UTF-8");
$xml_writer->startTag("data");

my %not_found_skills = ();

my $all_data;
{ local $/; $all_data = <>; }
$all_data =~ s/[\n\r]//g;
while ($all_data =~ /var og=\[([^;]*)\];/g) {
	my $json = "[$1]";
	$json =~ s/(\w+):/\"$1\":/g;
	my $data = decode_json($json);
	# print Dumper($data);
	for my $elt (@{$data}) {
		$xml_writer->startTag("item");
		$xml_writer->dataElement("type", $part_name{$elt->{"part"}});
		$xml_writer->dataElement("name", mod_name($elt->{"name"}));
		$xml_writer->dataElement("defense", $elt->{"mindef"}) if ($elt->{"mindef"});
		$xml_writer->dataElement("defense_max", $elt->{"maxdef"}) if ($elt->{"maxdef"});
		if ($elt->{"slots"}) {
			$xml_writer->startTag("slots");
			for my $slot (@{$elt->{"slotlevels"}}) {
				$xml_writer->dataElement("slot", $slot) if ($slot);
			}
			$xml_writer->endTag();
		}
		my $has_resistance = 0;
		for (my $i = 0; $i < 5; ++$i) {
			$has_resistance = 1 if ($elt->{"resist"}[$i]);
		}
		if ($has_resistance) {
			$xml_writer->startTag("resistance");
			for (my $i = 0; $i < 5; ++$i) {
				$xml_writer->dataElement($resist_name{$i}, $elt->{"resist"}[$i]);
			}
			$xml_writer->endTag();
		}

		my $has_buff_refs = 0;
		for my $skill (keys %{$elt->{"skills"}}) {
			my $id = $skill;
			$id =~ s/\x{2019}/'/g;
			$id =~ s/\s*$//g;
			if ($buff_name_id{$id}) {
				$id = $buff_name_id{$id};
				my $level = $elt->{"skills"}{$skill};
				unless ($has_buff_refs) {
					$xml_writer->startTag("buff_refs");
					$has_buff_refs = 1;
				}
				if ($level > 1) {
					$xml_writer->emptyTag("buff_ref", "id" => $id, "level" => $level);
				} else {
					$xml_writer->emptyTag("buff_ref", "id" => $id);
				}
			} else {
				unless ($set_bonus_name_id{$id}) {
					$not_found_skills{$skill} = 1;
					print STDERR "unknown skill: \"$id\"\n";
				}
			}
		}
		$xml_writer->endTag() if ($has_buff_refs);

		my $has_set_bonus_refs = 0;
		for my $skill (keys %{$elt->{"skills"}}) {
			my $id = $skill;
			$id =~ s/\x{2019}/'/g;
			$id =~ s/\s*$//g;
			if ($set_bonus_name_id{$id}) {
				$id = $set_bonus_name_id{$id};
				unless ($has_set_bonus_refs) {
					$xml_writer->startTag("set_bonus_refs");
					$has_set_bonus_refs = 1;
				}
				$xml_writer->emptyTag("set_bonus_ref", "id" => $id);
			}
		}
		$xml_writer->endTag() if ($has_set_bonus_refs);

		if ($elt->{"rare"}) {
			$xml_writer->dataElement("rare", $elt->{"rare"});
		}
		$xml_writer->endTag();
	}
}

my %set_bonuses = ();
my @set_bonus_list = ();
while ($all_data =~ /k.j=\[([^;]*)\];/g) {
	my $json = "[$1]";
	$json =~ s/(\w+):/\"$1\":/g;
	my $data = decode_json($json);
	#print Dumper($data);
	#next;
	for my $elt (@{$data}) {
		for my $skill (@{$elt->{"skills"}}) {
			my $name = $skill->{"skill"};
			next unless ($not_found_skills{$name});
			unless ($set_bonuses{$name}) {
				$set_bonuses{$name} = {};
				push @set_bonus_list, $name;
			}
			$set_bonuses{$name}->{$skill->{"points"}} ||= [];
			push @{$set_bonuses{$name}->{$skill->{"points"}}}, $skill;
		}
	}
}

for my $set_bonus_name (@set_bonus_list) {
	$xml_writer->startTag("set_bonus", "id" => name_to_id($set_bonus_name));
	$xml_writer->dataElement("name", $set_bonus_name);
	for my $level (sort keys %{$set_bonuses{$set_bonus_name}}) {
		for my $skill (@{$set_bonuses{$set_bonus_name}->{$level}}) {
			$xml_writer->startTag("set_bonus_level", "level" => $level);
			my $n = $skill->{"name"};
			$n =~ s/\x{2019}/'/g;
			$n =~ s/\s*\([^(]*\)\s*$//;
			if ($buff_name_id{$n}) {
				$xml_writer->emptyTag("buff_ref", "id" => $buff_name_id{$n});
			} else {
				print STDERR "unknown skill: $n\n";
			}
			$xml_writer->endTag();
		}
	}
	$xml_writer->endTag();
}

$xml_writer->endTag();
$xml_writer->end();
