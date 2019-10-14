#!/usr/bin/perl

use strict;
use warnings;

use utf8;
#use open ':std', ':encoding(UTF-8)';

use XML::Writer;
use JSON;
use Data::Dumper;

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

my $xml_writer;
my $json_data = {};

for my $file (@ARGV) {
#	open my $fh, "<:encoding(utf8)", $file || die "$file: $!";
	open my $fh, "<", $file || die "$file: $!";
	my $all_data_text;
	{ local $/; $all_data_text = <$fh>; }
	my $jd = decode_json($all_data_text);
	for my $k (sort keys %{$jd}) {
		$json_data->{$k} ||= {};
		for my $sk (sort keys %{$jd->{$k}}) {
			my %main_kmap = (
				"2" => "WeaponType",
				"3" => "WeaponRaw",
				"4" => "WeaponElement",
				"5" => "WeaponElemental",
				"6" => "WeaponElementalHidden",
				"7" => "WeaponAffinity",
				"8" => "WeaponDefenseBonus",
				"9" => "WeaponRarity",
				"10" => "WeaponSlots",
				"11" => "Sharpness",
				"12" => "Handycraftneeded"
			);
			if ($jd->{$k}{2} || $jd->{$k}{"WeaponType"}) {
				if ($main_kmap{$sk}) {
					$json_data->{$k}{$main_kmap{$sk}} = $jd->{$k}{$sk};
				} else {
					$json_data->{$k}{$sk} = $jd->{$k}{$sk};
				}
			} else {
				$json_data->{$k}{"x_$sk"} = $jd->{$k}{$sk};
			}
		}
	}
#	print $all_data_text;
}

# print Dumper($json_data);

my $io = IO::Handle->new();
$io->fdopen(fileno(STDOUT), ">");
$xml_writer = XML::Writer->new(OUTPUT => $io, ENCODING => 'utf-8', DATA_MODE => 1, DATA_INDENT => 4);
$xml_writer->xmlDecl("UTF-8");
$xml_writer->startTag("data");

my @special_fields = ("Phial", "Special", "SpecMechanics", "Special2");

for my $weap_name (keys %{$json_data}) {
	my $weap = $json_data->{$weap_name};
	next unless $weap->{"WeaponType"};

	$xml_writer->startTag("weapon");
	my $weap_type = lc($weap->{"WeaponType"});
	$weap_type =~ s/\s+/_/g;
	$xml_writer->emptyTag("weapon_type_ref", "id" => $weap_type);

	$weap_name =~ s/'(\w+)'/"$1"/;
	$xml_writer->dataElement("name", $weap_name);

	$xml_writer->dataElement("inflated_attack", $weap->{"WeaponRaw"});
	$xml_writer->dataElement("attack", $weap->{"WeaponRaw"} / $inflate_by_type{$weap_type});
	$xml_writer->dataElement("affinity", $weap->{"WeaponAffinity"}) if ($weap->{"WeaponAffinity"});
	$xml_writer->dataElement("defense", $weap->{"WeaponDefenseBonus"}) if ($weap->{"WeaponDefenseBonus"});
	$xml_writer->dataElement("awakened", "true") if ($weap->{"WeaponElementalHidden"});

	my $has_elements = 0;
	for my $elt (split /\+/, lc($weap->{"WeaponElement"})) {
		if ($elt && $elt ne "none") {
			unless ($has_elements) {
				$xml_writer->startTag("element");
				$has_elements = 1;
			}
			$xml_writer->dataElement("inflated_$elt", $weap->{"WeaponElemental"});
			$xml_writer->dataElement("$elt", $weap->{"WeaponElemental"} / 10);
		}
	}
	$xml_writer->endTag() if ($has_elements);

	my $has_phial = 0;
	if ($weap_type eq "switch_axe" || $weap_type eq "charge_blade") {
		for my $field (@special_fields) {
			my $field_val = $weap->{$field};
			next unless ($field_val);
			if ($field_val =~ /(\w+),(\d+)/) {
				my $elt = lc($1);
				$elt = "exhaust" if ($elt eq "exhaus");
				my $elt_level = $2;
				unless ($has_phial) {
					$xml_writer->startTag("phial");
					$has_phial = 1;
				}
				$xml_writer->dataElement("inflated_$elt", $elt_level);
				$xml_writer->dataElement("$elt", $elt_level / 10);
			} elsif ($field_val =~ /(Power Element|Power|Impact)/) {
				my %phmap = (
					"Power Element" => "element",
					"Power" => "power",
					"Impact" => "impact");
				unless ($has_phial) {
					$xml_writer->startTag("phial");
					$has_phial = 1;
				}
				$xml_writer->emptyTag($phmap{$1});
			}
		}
	}
	$xml_writer->endTag() if ($has_phial);

	if ($weap->{"x_Handycraftneeded"}) {
		my @sh = (split /,/, $weap->{"x_Sharpness"});
		my @sh_plus = (split /,/, $weap->{"x_Handycraftneeded"});
		shift @sh;
		shift @sh_plus;
		$xml_writer->startTag("sharpness");
		my $sh_total = 0;
		my $shp_total = 0;
		for my $sharpness_color ("red", "orange", "yellow", "green", "blue", "white", "purple") {
			my $shv = shift @sh;
			my $shvp = shift @sh_plus;
			if ($shvp) {
				$xml_writer->dataElement($sharpness_color, $shvp);
				$sh_total += $shv;
				$shp_total += $shvp;
			}
		}
		$xml_writer->dataElement("plus", $shp_total - $sh_total) if ($shp_total - $sh_total);
		$xml_writer->endTag();
	}

	my $has_slots = 0;
	for my $slot_level (split //, $weap->{"WeaponSlots"}) {
		next unless ($slot_level);
		unless ($has_slots) {
			$xml_writer->startTag("slots");
			$has_slots = 1;
		}
		$xml_writer->dataElement("slot", $slot_level);
	}
	$xml_writer->endTag() if ($has_slots);

	my $rare = $weap->{"WeaponRarity"};
	$rare =~ s/\|.*//;
	$xml_writer->dataElement("rare", $rare) if ($rare);

	$xml_writer->endTag();
}

$xml_writer->endTag();
$xml_writer->end();
