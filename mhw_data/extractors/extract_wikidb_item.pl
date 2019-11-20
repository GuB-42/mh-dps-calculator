#!/usr/bin/perl

# http://mhw.wiki-db.com/sim/compiled/sim-compiled.js
# http://mhw.wiki-db.com/sim/compiled/sim-compiled-en.js

use strict;
use warnings;

use utf8;
use open ':std', ':encoding(UTF-8)';

use XML::Writer;
use XML::Parser;
use JSON;
use Data::Dumper;

my $xml_writer;

my %rares = (
	"antidote_jewel_1" => 5,
	"geology_jewel_1" => 5,
	"heavy_artillery_jewel_1" => 5,
	"hungerless_jewel_1" => 5,
	"satiated_jewel_1" => 5,
	"protection_jewel_1" => 5,
	"meowster_jewel_1" => 5,
	"botany_jewel_1" => 5,
	"tip_toe_jewel_1" => 5,
	"flight_jewel_2" => 5,
	"smoke_jewel_1" => 5,
	"mirewalker_jewel_1" => 5,
	"specimen_jewel_1" => 5,
	"miasma_jewel_1" => 5,
	"scent_jewel_1" => 5,
	"intimidator_jewel_1" => 5,
	"crisis_jewel_1" => 5,
	"antipara_jewel_1" => 5,
	"fortitude_jewel_1" => 5,
	"water_res_jewel_1" => 5,
	"suture_jewel_1" => 5,
	"dragon_res_jewel_1" => 5,
	"fire_res_jewel_1" => 5,
	"ice_res_jewel_1" => 5,
	"def_lock_jewel_1" => 5,
	"thunder_res_jewel_1" => 5,
	"pep_jewel_1" => 5,
	"antiblast_jewel_1" => 5,
	"gobbler_jewel_1" => 6,
	"physique_jewel_2" => 6,
	"blaze_jewel_1" => 6,
	"evasion_jewel_2" => 6,
	"jumping_jewel_2" => 6,
	"sheath_jewel_1" => 6,
	"friendship_jewel_1" => 6,
	"enduring_jewel_1" => 6,
	"medicine_jewel_1" => 6,
	"grinder_jewel_1" => 6,
	"steadfast_jewel_1" => 6,
	"bomber_jewel_1" => 6,
	"recovery_jewel_1" => 6,
	"slider_jewel_2" => 6,
	"defense_jewel_1" => 6,
	"stonethrower_jewel_1" => 6,
	"footing_jewel_2" => 6,
	"brace_jewel_3" => 6,
	"wind_resist_jewel_2" => 6,
	"earplug_jewel_3" => 6,
	"vitality_jewel_1" => 6,
	"resistor_jewel_1" => 6,
	"sonorous_jewel_1" => 6,
	"ko_jewel_2" => 6,
	"stream_jewel_1" => 6,
	"frost_jewel_1" => 6,
	"bolt_jewel_1" => 6,
	"dragon_jewel_1" => 6,
	"venom_jewel_1" => 6,
	"poisoncoat_jewel_3" => 6,
	"expert_jewel_1" => 6,
	"tenderizer_jewel_2" => 6,
	"destroyer_jewel_2" => 6,
	"elementless_jewel_2" => 6,
	"drain_jewel_1" => 6,
	"paracoat_jewel_3" => 7,
	"sharp_jewel_2" => 7,
	"shield_jewel_2" => 7,
	"dragonseal_jewel_3" => 7,
	"trueshot_jewel_1" => 7,
	"enhancer_jewel_2" => 7,
	"forceshot_jewel_3" => 7,
	"furor_jewel_2" => 7,
	"potential_jewel_2" => 7,
	"paralyzer_jewel_1" => 7,
	"sleep_jewel_1" => 7,
	"ironwall_jewel_1" => 7,
	"refresh_jewel_2" => 7,
	"blast_jewel_1" => 7,
	"sleepcoat_jewel_3" => 7,
	"blastcoat_jewel_3" => 7,
	"throttle_jewel_2" => 7,
	"challenger_jewel_2" => 7,
	"attack_jewel_1" => 7,
	"flawless_jewel_2" => 7,
	"magazine_jewel_2" => 7,
	"mighty_jewel_2" => 7,
	"sprinter_jewel_2" => 8,
	"critical_jewel_2" => 8,
	"artillery_jewel_1" => 8,
	"fungiform_jewel_1" => 8,
	"release_jewel_3" => 8,
	"spread_jewel_3" => 8,
	"charger_jewel_2" => 8,
	"handicraft_jewel_3" => 8,
	"mighty_bow_jewel_2" => 8,
	"minds_eye_jewel_2" => 8,
	"draw_jewel_2" => 8,
	"pierce_jewel_3" => 8,
);

my %buff_name_id = (
	"Defence Boost" => "defense_boost",
	"Paralysis Functionality" => "para_functionality",
	"Survival Expert" => "sporepuff_expert",
	"Effluvial Resistance" => "effluvia_resistance",
	"Scent Hound" => "scenthound",
	"Aquatic/Polar Mobility" => "aquatic_expert",
	"環境利用の知識" => "sporepuff_expert",
	"水場・深雪適応" => => "aquatic_expert",
);

my %set_bonus_name_id = (
	"Soul of the Dragon" => "soul_of_the_dragoon",
	"Val Hazak Vitality" => "vaal_hazak_vitality",
	"Legian Ambition" => "legiana_ambition",
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
	$n =~ s/'s/s/g;
	$n =~ s/[_\W]+/_/g;
	$n =~ s/^\s*|\s*$//;
	return lc($n);
}

my @xml_stack;
my @xml_string_stack;
my @xml_stack_attr;
my $cur_buff_id;

sub process_start
{
}

sub process_val
{
	if ($xml_stack[0] =~ /^name/ && $xml_stack[1] eq "buff_group") {
		$buff_name_id{$xml_string_stack[0]} = $xml_stack_attr[1]->{"id"};
	} elsif ($xml_stack[0] =~ /^name/ && $xml_stack[1] eq "set_bonus") {
		$set_bonus_name_id{$xml_string_stack[0]} = $xml_stack_attr[1]->{"id"};
	}
}

sub handle_start
{
	my ($expat, $element, @attr) = @_;
	unshift @xml_stack, $element;
	unshift @xml_string_stack, "";
	my $attr_map = {};
	for (my $i = 0; $i < @attr; $i += 2) {
		$attr_map->{$attr[$i]} = $attr[$i + 1];
	}
	unshift @xml_stack_attr, $attr_map;
	process_start();
}
sub handle_end
{
	my ($expat, $element) = @_;
	process_val();
	shift @xml_stack;
	shift @xml_string_stack;
	shift @xml_stack_attr;
}
sub handle_char
{
	my ($expat, $string) = @_;
	$xml_string_stack[0] .= $string;
}

my $buffs_file = shift @ARGV;
my $p = new XML::Parser(Handlers => {
	Start => \&handle_start,
	End => \&handle_end,
	Char => \&handle_char });
@xml_string_stack = ();
@xml_stack = ("null", "null", "null");
$p->parsefile($buffs_file);

my %not_found_skills = ();

sub decode_skills {
	my $xml_writer = $_[0];
	my $skill_map = $_[1];

	my $has_buff_refs = 0;
	for my $skill (keys %{$skill_map}) {
		my $id = $skill;
		$id =~ s/\x{2019}/'/g;
		$id =~ s/\s*$//g;
		if ($buff_name_id{$id}) {
			$id = $buff_name_id{$id};
			my $level = $skill_map->{$skill};
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
	for my $skill (keys %{$skill_map}) {
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
}

my $io = IO::Handle->new();
$io->fdopen(fileno(STDOUT), ">");
$xml_writer = XML::Writer->new(OUTPUT => $io, ENCODING => 'utf-8', DATA_MODE => 1, DATA_INDENT => 4);
$xml_writer->xmlDecl("UTF-8");
$xml_writer->startTag("data");

my $all_data;
{ local $/; $all_data = <>; }
$all_data =~ s/[\n\r]//g;

while ($all_data =~ /l\.X=\[([^;]*)\];/g) {
	my $json = "[$1]";
	$json =~ s/(\w+):/\"$1\":/g;
	my $data = decode_json($json);
	for my $elt (@{$data}) {
		next if ($elt->{"name"} =~ /^(?:Jewel \d|装飾品【\d】)$/);
		$xml_writer->startTag("item", "id" => name_to_id($elt->{"name"}));
		$xml_writer->dataElement("type", "decoration");
		$xml_writer->dataElement("name", $elt->{"name"});
		$xml_writer->dataElement("decoration_level", $elt->{"level"});
		if ($rares{name_to_id($elt->{"name"})}) {
			$xml_writer->dataElement("rare", $rares{name_to_id($elt->{"name"})});
		}
		decode_skills($xml_writer, $elt->{"skills"});
		$xml_writer->endTag();
	}
}

while ($all_data =~ /var qg=\[([^;]*)\];/g) {
	my $json = "[$1]";
	$json =~ s/(\w+):/\"$1\":/g;
	my $data = decode_json($json);
	for my $category (@{$data}) {
		for my $elt (@{$category->{"decos"}}) {
			$xml_writer->startTag("item", "id" => name_to_id($elt->{"name"}));
			$xml_writer->dataElement("type", "decoration");
			$xml_writer->dataElement("name", $elt->{"name"});
			$xml_writer->dataElement("decoration_level", 4);
			decode_skills($xml_writer, $elt->{"skills"});
			$xml_writer->endTag();
		}
	}
}

$xml_writer->endTag();
$xml_writer->end();
exit;

while ($all_data =~ /var pg=\[([^;]*)\];/g) {
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

		decode_skills($xml_writer, $elt->{"skills"});

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
