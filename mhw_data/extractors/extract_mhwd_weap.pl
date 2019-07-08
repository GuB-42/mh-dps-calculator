#!/usr/bin/perl

use strict;
use warnings;

use Text::CSV;
use XML::Writer;
use Data::Dumper;

my %type_rename = (
	"great-sword" => "great_sword",
	"long-sword" => "long_sword",
	"sword-and-shield" => "sword_and_shield",
	"dual-blades" => "dual_blades",
	"hammer" => "hammer",
	"hunting-horn" => "hunting_horn",
	"lance" => "lance",
	"gunlance" => "gunlance",
	"switch-axe" => "switch_axe",
	"charge-blade" => "charge_blade",
	"insect-glaive" => "insect_glaive",
	"light-bowgun" => "light_bowgun",
	"heavy-bowgun" => "heavy_bowgun",
	"bow" => "bow"
);

my %inflate_by_type = (
	"great_sword" => 4.8,
	"long_sword" => 3.3,
	"sword_and_shield" => 1.4,
	"dual_blades" => 1.4,
	"hammer" => 5.2,
	"hunting_horn" => 4.2,
	"lance" => 2.3,
	"gunlance" => 2.3,
	"switch_axe" => 3.5,
	"charge_blade" => 3.6,
	"insect_glaive" => 3.1,
	"light_bowgun" => 1.3,
	"heavy_bowgun" => 1.5,
	"bow" => 1.2
);

my %coating_map = (
	"bow_close" => "coating_close_range",
	"bow_power" => "coating_power",
	"bow_paralysis" => "coating_paralysis",
	"bow_poison" => "coating_poison",
	"bow_sleep" => "coating_sleep",
	"bow_blast" => "coating_blast"
);

my @weapons = ();
my %weap_map = ();
my %item_monsters = ();
my %monster_size = ();

my $io = IO::Handle->new();
$io->fdopen(fileno(STDOUT), "w");
my $xml_writer = XML::Writer->new(OUTPUT => $io, ENCODING => 'utf-8', DATA_MODE => 1, DATA_INDENT => 4);
$xml_writer->xmlDecl("UTF-8");

sub process_row {
	my %row = %{shift()};
	#print Dumper(\%row);

	my $weapon_type = $type_rename{$row{"weapon_type"}};
#          'id',
#          'name_en',
#          'weapon_type',
#          'previous_en',
#          'category',
#          'rarity',
#          'attack',
#          'affinity',
#          'defense',
#          'element_hidden',
#          'element1',
#          'element1_attack',
#          'element2',
#          'element2_attack',
#          'elderseal',
#          'slot_1',
#          'slot_2',
#          'slot_3',
#          'kinsect_bonus',
#          'phial',
#          'phial_power',
#          'shelling',
#          'shelling_level',
#          'notes',
#          'ammo_config',
#          'skill',

	$xml_writer->startTag("weapon");
	$xml_writer->emptyTag("weapon_type_ref", "id" => $weapon_type);

	$row{"name_en"} && $xml_writer->dataElement("name", $row{"name_en"});

	$xml_writer->dataElement("rare", $row{"rarity"});
	$xml_writer->dataElement("attack", $row{"attack"} / $inflate_by_type{$weapon_type});
	$xml_writer->dataElement("inflated_attack", $row{"attack"});
	$row{"affinity"} && $xml_writer->dataElement("affinity", $row{"affinity"});
	$row{"defense"} && $xml_writer->dataElement("defense", $row{"defense"});

	my $has_element = 0;
	for my $elt_id (1, 2) {
		my $elt_name = $row{"element${elt_id}"};
		my $elt_val = $row{"element${elt_id}_attack"};
		my %elt_map = (
			"Water" => "water",
			"Fire" => "fire",
			"Thunder" => "thunder",
			"Ice" => "ice",
			"Dragon" => "dragon",
			"Blast" => "blast",
			"Sleep" => "sleep",
			"Paralysis" => "paralysis",
			"Poison" => "poison"
		);
		if ($elt_map{$elt_name}) {
			unless ($has_element) {
				$xml_writer->startTag("element");
				$has_element = 1;
			}
			$xml_writer->dataElement("inflated_" . $elt_map{$elt_name}, $elt_val);
			$xml_writer->dataElement($elt_map{$elt_name}, $elt_val / 10);
		}
	}
	$xml_writer->endTag() if ($has_element);

	if (lc($row{'element_hidden'}) eq "true") {
		$xml_writer->dataElement("awakened", "true");
	}

	my $has_phial = 0;
	if ($row{"phial"}) {
		unless ($has_phial) {
			$xml_writer->startTag("phial");
			$has_phial = 1;
		}
		if ($row{"phial"} eq "impact") {
			$xml_writer->emptyTag("impact");
		} elsif ($row{"phial"} eq "power element") {
			$xml_writer->emptyTag("element");
		} elsif ($row{"phial"} eq "power") {
			$xml_writer->emptyTag("power");
		} else {
			$xml_writer->dataElement("inflated_" . $row{"phial"}, $row{"phial_power"});
			$xml_writer->dataElement($row{"phial"}, $row{"phial_power"} / 10);
		}
	}
	$xml_writer->endTag() if ($has_phial);

	my $has_ntoes = 0;
	if ($row{"notes"}) {
		unless ($has_ntoes) {
			$xml_writer->startTag("notes");
			$has_ntoes = 1;
		}
		for (split //, $row{"notes"}) {
			my %note_map = (
				"P" => "purple",
				"R" => "red",
				"B" => "blue",
				"W" => "white",
				"G" => "green",
				"Y" => "yellow",
				"O" => "orange",
				"C" => "light_blue"
			);
			$xml_writer->emptyTag($note_map{$_});
		}
	}
	$xml_writer->endTag() if ($has_ntoes);

	my $has_slots = 0;
	for my $slot_id (1, 2, 3) {
		if ($row{"slot_${slot_id}"}) {
			unless ($has_slots) {
				$xml_writer->startTag("slots");
				$has_slots = 1;
			}
			$xml_writer->dataElement("slot", $row{"slot_${slot_id}"});
		}
	}
	$xml_writer->endTag() if ($has_slots);

	my $has_sharpness = 0;
	for my $sharp_color ("red", "orange", "yellow", "green", "blue", "white", "purple") {
		next unless ($row{"sharpness_$sharp_color"});
		unless ($has_sharpness) {
			$xml_writer->startTag("sharpness");
			$has_sharpness = 1;
		}
		$xml_writer->dataElement($sharp_color, $row{"sharpness_$sharp_color"});
	}
	if ($has_sharpness) {
		if (lc($row{"sharpness_maxed"}) eq "false") {
			$xml_writer->dataElement("plus", 50);
		}
		$xml_writer->endTag()
	}

	my $has_ammos = 0;
	for my $coat ("bow_close", "bow_power", "bow_paralysis", "bow_poison", "bow_sleep", "bow_blast") {
		if (defined $row{$coat} && lc($row{$coat}) eq "true") {
			unless ($has_ammos) {
				$xml_writer->startTag("ammos");
				$has_ammos = 1;
			}
			$xml_writer->emptyTag("ammo_ref", "id" => $coating_map{$coat});
		}
	}
	if ($row{"shelling"}) {
		unless ($has_ammos) {
			$xml_writer->startTag("ammos");
			$has_ammos = 1;
		}
		$xml_writer->emptyTag("ammo_ref", "id" => "shell_$row{shelling}_$row{shelling_level}");
	}
	$xml_writer->endTag() if ($has_ammos);

#	my $has_from_monsters = 0;
#	my %from_monsters = ();
#	my %from_monsters_inv = ();
#	my $item_count = 0;
#	for my $item_id (1 .. 4) {
#		if (defined $row{"item${item_id}_name"}) {
#			++$item_count;
#			my $item = $row{"item${item_id}_name"};
#			my $cnt = scalar keys %{$item_monsters{$item}};
#			for my $m (keys %{$item_monsters{$item}}) {
#				$from_monsters{$m} ||= 0;
#				$from_monsters{$m} += 1 / $cnt;
#			}
#		}
#	}
#	for my $m (keys %from_monsters) {
#		$from_monsters_inv{$from_monsters{$m}} ||= [];
#		push @{$from_monsters_inv{$from_monsters{$m}}}, $m;
#	}
#	my $first = 1;
#	for my $val (sort { $b <=> $a } keys %from_monsters_inv) {
#		for my $m (@{$from_monsters_inv{$val}}) {
#			print "$m\n" unless ($monster_size{$m});
#			if ($monster_size{$m} eq "large" &&
#			    (($from_monsters{$m} >= 0.25 * $item_count && $first) ||
#			     $from_monsters{$m} >= 0.35 * $item_count)) {
#				unless ($has_from_monsters) {
#					$xml_writer->startTag("from_monsters");
#					$has_from_monsters = 1;
#				}
#				$xml_writer->emptyTag("monster_ref", "id" => $m, "val" => $val);
#			}
#		}
#		$first = 0;
#	}
#	$xml_writer->endTag() if ($has_from_monsters);
#	#unless ($has_from_monsters) {
#		print STDERR $row{"name_en"} . "\n";
#		for my $val (sort { $b <=> $a } keys %from_monsters_inv) {
#			for my $m (@{$from_monsters_inv{$val}}) {
#				print STDERR "- $m : $val\n";
#			}
#		}
#		print STDERR "\n";
#	#}

	$xml_writer->endTag();
}

for my $file (@ARGV) {
	my $csv = Text::CSV->new( { binary => 1 } ) ||
		die "Cannot use CSV: ".Text::CSV->error_diag();
	open my $fh, "<:encoding(utf8)", $file || die "$file: $!";
	my $first = 1;
	my @header = ();
	while (my $row = $csv->getline($fh)) {
		if ($first) {
			@header = @{$row};
			$first = 0;
		} else {
			my $hline = {};
			for (my $i = 0; $i < scalar @{$row} && $i < scalar @header; ++$i) {
				$hline->{$header[$i]} = $row->[$i];
			}
			if (defined $hline->{"condition_en"}) {
				$item_monsters{$hline->{"item_en"}} ||= {};
				$item_monsters{$hline->{"item_en"}}->{$hline->{"base_name_en"}} = 1;
			} elsif (defined $hline->{"size"}) {
				$monster_size{$hline->{"name_en"}} = $hline->{"size"};
			} else {
				my $name = $hline->{"name_en"} || $hline->{"base_name_en"};
				unless (defined $weap_map{$name}) {
					$weap_map{$name} ||= {};
					push @weapons, $weap_map{$name};
				}
				my $prefix = "";
				if (defined $hline->{"blast"}) {
					$prefix = "bow_";
				} elsif (defined $hline->{"white"}) {
					$prefix = "sharpness_";
				}
				for my $k (keys %{$hline}) {
					$weap_map{$name}->{"$prefix$k"} = $hline->{$k};
				}
			}
		}
	}
	$csv->eof || $csv->error_diag();
	close $fh;
}

$xml_writer->startTag("data");
process_row($_) for (@weapons);
$xml_writer->endTag();
$xml_writer->end();

