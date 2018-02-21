#!/usr/bin/perl

use strict;
use warnings;

use XML::Writer;
use HTML::Parser;
use HTML::Entities;

my $xml_writer;

my @data_row = ();
my $weapon_type = "";
my %sharpness = ();
my @slots = ();

sub process_data_row {
	$xml_writer->startTag("weapon");
	$xml_writer->dataElement("type", $weapon_type);

	my $creatable = 0;
	my $name = $data_row[0];
	if ($name =~ /\(creatable\)/) {
		$name =~ s/\s*\(creatable\)//;
		$creatable = 1;
	}
	$xml_writer->dataElement("name", $name);

	$xml_writer->dataElement("inflated_attack", $data_row[1]);
	$xml_writer->dataElement("attack", $data_row[2]);
	if ($data_row[3] =~ /Affinity\s+\+?(-?\d+)/) {
		$xml_writer->dataElement("affinity", $1);
	}

	if ($data_row[3] =~ /\(/) {
		$xml_writer->dataElement("awakened", "true");
	}

	my $has_element = 0;
	while ($data_row[3] =~ /(\d+)\s+(\w+)/g) {
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
		if ($elt_map{$2}) {
			unless ($has_element) {
				$xml_writer->startTag("element");
				$has_element = 1;
			}
			$xml_writer->dataElement("inflated_" . $elt_map{$2}, $1);
			$xml_writer->dataElement($elt_map{$2}, $1 / 10);
		}
	}
	$xml_writer->endTag() if ($has_element);

	my $has_phial = 0;
	if ($data_row[3] =~ /(\w+)\s+Phial(?:\s+(\d+))?/g) {
		my %elt_map = (
			"Water" => "water",
			"Fire" => "fire",
			"Thunder" => "thunder",
			"Ice" => "ice",
			"Dragon" => "dragon",
			"Blast" => "blast",
			"Sleep" => "sleep",
			"Paralysis" => "paralysis",
			"Para" => "paralysis",
			"Poison" => "poison",
			"Exhaust" => "exhaust",
			"Impact" => "impact",
			"Power" => "power",
			"Element" => "element",
		);
		if ($elt_map{$1}) {
			unless ($has_phial) {
				$xml_writer->startTag("phial");
				$has_phial = 1;
			}
			if ($2) {
				$xml_writer->dataElement("inflated_" . $elt_map{$1}, $2);
				$xml_writer->dataElement($elt_map{$1}, $2 / 10);
			} else {
				$xml_writer->emptyTag($elt_map{$1});
			}

		}
	}
	$xml_writer->endTag() if ($has_phial);

	my $has_sharpness = 0;
	for my $sharpness_color ("red", "orange", "yellow", "green", "blue", "white", "purple") {
		if ($sharpness{$sharpness_color}) {
			unless ($has_sharpness) {
				$xml_writer->startTag("sharpness");
				$has_sharpness = 1;
			}
			$xml_writer->dataElement($sharpness_color, $sharpness{$sharpness_color});
		}
	}
	$xml_writer->endTag() if ($has_sharpness);

	my $has_slots = 0;
	for my $slot_level (@slots) {
		unless ($has_slots) {
			$xml_writer->startTag("slots");
			$has_slots = 1;
		}
		$xml_writer->dataElement("slot", $slot_level);
	}
	$xml_writer->endTag() if ($has_slots);

	if ($data_row[6] =~ /(\d+)/) {
		$xml_writer->dataElement("rare", $1);
	}

	$xml_writer->dataElement("creatable", "true") if ($creatable);

	$xml_writer->endTag();
}

my $in_tr = 0;
my $in_td = 0;
my $cur_col = 0;
my $text_acc = "";

sub start {
	my ($self, $tag, $attr, $attrseq, $origtext) = @_;
	if (lc($tag) eq "td") {
		$in_td = 1;
		$text_acc = "";
	} elsif (lc($tag) eq "tr") {
		$in_tr = 0;
		$in_td = 0;
		$cur_col = 0;
		@data_row = ();
		%sharpness = ();
		@slots = ();
	} elsif (lc($tag) eq "div") {
		my $class = $attr->{"class"};
		if ($class) {
			my $style = $attr->{"style"};
			if ($class =~ /sharpness-(\w+)/) {
				my $sharpness_color = $1;
				if ($style =~ /(\d+)px/) {
					$sharpness{$sharpness_color} = $1;
				}
			}
		}
	} elsif (lc($tag) eq "i") {
		my $class = $attr->{"class"};
		if ($class) {
			if ($class =~ /zmdi-n-(\d+)/) {
				push @slots, $1;
			}
		}
	}
}

sub end {
	my ($self, $tag, $origtext) = @_;
	if (lc($tag) eq "td") {
		$in_td = 0;
		my $t = $text_acc;
		$t =~ s/&#(\d+);/$1 == 9495 ? "" : chr($1)/eg;
		$t = decode_entities($t);
		$t =~ s/\s+/ /g;
		$t =~ s/^\s+//;
		$t =~ s/\s+$//;
		$data_row[$cur_col] = $t;
#		print "$cur_col: $t\n";
		++$cur_col;
	} elsif (lc($tag) eq "tr" && (@data_row > 0)) {
		process_data_row();
		$in_tr = 0;
	}
}

sub text {
	my ($self, $text) = @_;
	$text_acc .= $text;
}

my $io = IO::Handle->new();
$io->fdopen(fileno(STDOUT), "w");
$xml_writer = XML::Writer->new(OUTPUT => $io, ENCODING => 'utf-8', DATA_MODE => 1, DATA_INDENT => 4);
$xml_writer->xmlDecl("UTF-8");
$xml_writer->startTag("data");

for my $file (@ARGV) {
	$weapon_type = $file;
	$weapon_type =~ s/.*\///;
	$weapon_type =~ s/-/_/g;
	$weapon_type = "sword_and_shield" if ($weapon_type eq "sword");
	my $p = HTML::Parser->new(api_version => 3,
	                          start_h => [ \&start, "self, tagname, attr, attrseq, text" ],
	                          end_h => [ \&end, "self, tagname, text" ],
	                          text_h => [ \&text, "self, text, is_cdata" ]);
	$p->parse_file($file);
}

$xml_writer->endTag();
$xml_writer->end();
# print "$xml_writer\n";
