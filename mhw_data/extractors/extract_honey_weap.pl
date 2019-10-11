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
	for my $k (keys %{$jd}) {
		$json_data->{$k} ||= {};
		for my $sk (keys %{$jd->{$k}}) {
			$json_data->{$k}{$sk} = $jd->{$k}{$sk};
		}
	}
#	print $all_data_text;
}

#print Dumper($json_data);

my $io = IO::Handle->new();
$io->fdopen(fileno(STDOUT), ">");
$xml_writer = XML::Writer->new(OUTPUT => $io, ENCODING => 'utf-8', DATA_MODE => 1, DATA_INDENT => 4);
$xml_writer->xmlDecl("UTF-8");
$xml_writer->startTag("data");

for my $weap_name (keys %{$json_data}) {
	my $weap = $json_data->{$weap_name};
	next unless $weap->{2};
	#next unless $weap->{"Handycraftneeded"};

	$xml_writer->startTag("weapon");
	my $weap_type = lc($weap->{2});
	$weap_type =~ s/\s+/_/g;
	$xml_writer->emptyTag("weapon_type_ref", "id" => $weap_type);

	$weap_name =~ s/'(\w+)'/"$1"/;
	$xml_writer->dataElement("name", $weap_name);

	$xml_writer->dataElement("inflated_attack", $weap->{3});
	$xml_writer->dataElement("attack", $weap->{3} / $inflate_by_type{$weap_type});
	$xml_writer->dataElement("affinity", $weap->{7}) if ($weap->{7});
	$xml_writer->dataElement("defense", $weap->{8}) if ($weap->{8});
	$xml_writer->dataElement("awakened", "true") if ($weap->{6});

	my $has_elements = 0;
	for my $elt (split /\+/, lc($weap->{4})) {
		if ($elt && $elt ne "none") {
			unless ($has_elements) {
				$xml_writer->startTag("element");
				$has_elements = 1;
			}
			$xml_writer->dataElement("inflated_$elt", $weap->{5});
			$xml_writer->dataElement("$elt", $weap->{5} / 10);
		}
	}
	$xml_writer->endTag() if ($has_elements);

	if ($weap->{"Handycraftneeded"}) {
		my @sh = (split /,/, $weap->{"Sharpness"});
		my @sh_plus = (split /,/, $weap->{"Handycraftneeded"});
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
	for my $slot_level (split //, $weap->{10}) {
		next unless ($slot_level);
		unless ($has_slots) {
			$xml_writer->startTag("slots");
			$has_slots = 1;
		}
		$xml_writer->dataElement("slot", $slot_level);
	}
	$xml_writer->endTag() if ($has_slots);

	my $rare = $weap->{9};
	$rare =~ s/\|.*//;
	$xml_writer->dataElement("rare", $rare) if ($rare);

	$xml_writer->endTag();
}

$xml_writer->endTag();
$xml_writer->end();
