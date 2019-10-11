#!/usr/bin/perl

use strict;
use warnings;

use XML::Writer;
use HTML::Parser;
use HTML::Entities;

my $xml_writer;

my @data_row = ();
my $weapon_type = "";
my $name_key = "name";
my %sharpness = ();
my %sharpness_plus = ();
my @ammos = ();
my $is_iceborne = 0;

sub process_data_row {
	$xml_writer->startTag("weapon");
	$xml_writer->emptyTag("weapon_type_ref", "id" => $weapon_type);

	$xml_writer->dataElement($name_key, $data_row[0]);

	if ($data_row[1] =~ /^(\d+)/) {
		$xml_writer->dataElement("inflated_attack", $1);
	}
	if ($data_row[1] =~ /\[(\d+)\]/) {
		$xml_writer->dataElement("attack", $1);
	}
	if ($data_row[2] =~ /\{affinity\}\s*\+?(-?\d+)/) {
		$xml_writer->dataElement("affinity", $1);
	}

	if ($data_row[2] =~ /\(/) {
		$xml_writer->dataElement("awakened", "true");
	}

	my $has_element = 0;
	while ($data_row[2] =~ /\{(fire|water|ice|thunder|dragon|poison|paralysis|sleep|blast|stun)\}\s*\(?\s*(\d+)/g) {
		unless ($has_element) {
			$xml_writer->startTag("element");
			$has_element = 1;
		}
		$xml_writer->dataElement("inflated_" . $1, $2);
		$xml_writer->dataElement($1, $2 / 10);
	}
	$xml_writer->endTag() if ($has_element);

	if ($data_row[2] =~ /\{elderseal\}\s*(\w+)/) {
		$xml_writer->dataElement("elderseal", lc($1));
	}

	if ($data_row[2] =~ /\{defense\}\s*\+?(-?\d+)/) {
		$xml_writer->dataElement("defense", $1);
	}

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
			my $phial_type = $elt_map{$1};
			my $phial_power = $2;
			if ($weapon_type eq "charge_blade") {
				if ($phial_type eq "power") {
					$phial_type = "impact";
					undef $phial_power;
				} elsif ($phial_type eq "element") {
					undef $phial_power;
				}
			}
			if ($phial_power) {
				$xml_writer->dataElement("inflated_" . $phial_type, $phial_power * 10);
				$xml_writer->dataElement($phial_type, $phial_power);
			} else {
				$xml_writer->emptyTag($phial_type);
			}

		}
	}
	$xml_writer->endTag() if ($has_phial);

	my $has_sharpness = 0;
	my $sharpness_total = 0;
	my $sharpness_plus_total = 0;
	for my $sharpness_color ("red", "orange", "yellow", "green", "blue", "white", "purple") {
		if ($sharpness{$sharpness_color}) {
			my $v = int($sharpness{$sharpness_color} * 0.4 + 0.5) * 10;
			$sharpness_total += $v;
		}
		if ($sharpness_plus{$sharpness_color}) {
			unless ($has_sharpness) {
				$xml_writer->startTag("sharpness");
				$has_sharpness = 1;
			}
			my $v = int($sharpness_plus{$sharpness_color} * 0.4 + 0.5) * 10;
			$xml_writer->dataElement($sharpness_color, $v);
			$sharpness_plus_total += $v;
		}
	}
	if ($has_sharpness) {
		$xml_writer->dataElement("plus", $sharpness_plus_total - $sharpness_total);
		$xml_writer->endTag()
	}

	my $has_ammos = 0;
	for my $a (@ammos) {
		unless ($has_ammos) {
			$xml_writer->startTag("ammos");
			$has_ammos = 1;
		}
		$xml_writer->emptyTag("ammo_ref", "id" => $a);
	}
	if ($data_row[3] =~ /(Normal|Wide|Long)\s+Lv\s*(\d+)/g) {
		unless ($has_ammos) {
			$xml_writer->startTag("ammos");
			$has_ammos = 1;
		}
		my $shell = lc($1);
		if ($shell eq "long") {
			$shell = "wide";
		} elsif ($shell eq "wide") {
			$shell = "long";
		}
		$xml_writer->emptyTag("ammo_ref", "id" => ("shell_${shell}_$2"));
	}
	$xml_writer->endTag() if ($has_ammos);

	if ($data_row[3] =~ /\{notes\}\s+(\d+)\s+(\d+)\s+(\d+)/) {
		$xml_writer->startTag("notes");
		for my $n ($1, $2, $3) {
			my %note_map = (
				"0" => "purple",
				"1" => "red",
				"2" => "orange",
				"3" => "yellow",
				"4" => "green",
				"5" => "blue",
				"6" => "light_blue",
				"7" => "white"
			);
			$xml_writer->emptyTag($note_map{$n});
		}
		$xml_writer->endTag();
	}

	my $has_slots = 0;
	while ($data_row[4] =~ /\{slot_(\d+)\}/g) {
		unless ($has_slots) {
			$xml_writer->startTag("slots");
			$has_slots = 1;
		}
		$xml_writer->dataElement("slot", $1);
	}
	$xml_writer->endTag() if ($has_slots);

	$xml_writer->startTag("categories");
	$xml_writer->emptyTag("category_ref", "id" => "iceborne") if ($is_iceborne);
	$xml_writer->endTag();

	$xml_writer->endTag();
}

my $in_subtable = 0;
my $in_td = 0;
my $in_div = 0;
my $in_div_class = "";
my $cur_col = 0;
my $text_acc = "";
my $text_acc_div = "";

sub start {
	my ($self, $tag, $attr, $attrseq, $origtext) = @_;
	if (lc($tag) eq "table") {
		my $class = $attr->{"class"};
		if ($class && $class eq "m-0") {
			$in_subtable = 1;
		}
	}
	return if $in_subtable;

	if (lc($tag) eq "td") {
		$in_td = 1;
		$text_acc = "";
	} elsif (lc($tag) eq "tr") {
		$in_td = 0;
		$cur_col = 0;
		@data_row = ();
		%sharpness = ();
		%sharpness_plus = ();
		@ammos = ();
		$is_iceborne = 0;
	} elsif (lc($tag) eq "div") {
		$in_div = 1;
		$in_div_class = $attr->{"class"};
		$text_acc_div = "";
		my $class = $attr->{"class"};
		if ($class) {
			my $style = $attr->{"style"};
			if ($class =~ /sharpness-(\w+)/) {
				my $sharpness_color = $1;
				if ($style =~ /(\d+\.?|\d*\.\d+)px/) {
					unless (defined $sharpness{$sharpness_color}) {
						$sharpness{$sharpness_color} = $1
					}
					$sharpness_plus{$sharpness_color} = $1;
				}
			}
		}
	} elsif  (lc($tag) eq "h6") {
		$text_acc = "";
	} elsif  (lc($tag) eq "rt") {
		$text_acc .= '[';
	} elsif (lc($tag) eq "img") {
		my %img_map = (
			"/storage/mhw/icon/affinity.png" => "affinity",
			"/storage/mhw/icon/defense.png" => "defense",
			"/storage/mhw/icon/deviation.png" => "deviation",
			"/storage/mhw/icon/elderseal.png" => "elderseal",
			"/storage/mhw/icon/element_1.png" => "fire",
			"/storage/mhw/icon/element_2.png" => "water",
			"/storage/mhw/icon/element_3.png" => "ice",
			"/storage/mhw/icon/element_4.png" => "thunder",
			"/storage/mhw/icon/element_5.png" => "dragon",
			"/storage/mhw/icon/element_6.png" => "poison",
			"/storage/mhw/icon/element_7.png" => "paralysis",
			"/storage/mhw/icon/element_8.png" => "sleep",
			"/storage/mhw/icon/element_9.png" => "blast",
			"/storage/mhw/icon/element_10.png" => "stun",
			"/storage/mhw/icon/element_11.png" => "element_11",
			"/storage/mhw/icon/element_12.png" => "element_12",
			"/storage/mhw/icon/ib_icon.png" => "iceborne",
			"/storage/mhw/icon/notes.png" => "notes",
			"/storage/mhw/icon/phial.png" => "phial",
			"/storage/mhw/icon/shelling.png" => "shelling",
			"/storage/mhw/icon/slot_size_1.png" => "slot_1",
			"/storage/mhw/icon/slot_size_2.png" => "slot_2",
			"/storage/mhw/icon/slot_size_3.png" => "slot_3",
			"/storage/mhw/icon/slot_size_4.png" => "slot_4",
			"/storage/mhw/icon/weapon_type_0.png" => "great_sword",
			"/storage/mhw/icon/weapon_type_1.png" => "sword_and_shield",
			"/storage/mhw/icon/weapon_type_2.png" => "dual_blades",
			"/storage/mhw/icon/weapon_type_3.png" => "long_sword",
			"/storage/mhw/icon/weapon_type_4.png" => "hammer",
			"/storage/mhw/icon/weapon_type_5.png" => "hunting_horn",
			"/storage/mhw/icon/weapon_type_6.png" => "lance",
			"/storage/mhw/icon/weapon_type_7.png" => "gunlance",
			"/storage/mhw/icon/weapon_type_8.png" => "switch_axe",
			"/storage/mhw/icon/weapon_type_9.png" => "charge_blade",
			"/storage/mhw/icon/weapon_type_10.png" => "insect_glaive",
			"/storage/mhw/icon/weapon_type_11.png" => "bow",
			"/storage/mhw/icon/weapon_type_12.png" => "heavy_bowgun",
			"/storage/mhw/icon/weapon_type_13.png" => "light_bowgun"
		);
		my $src = $attr->{"src"};
		if ($img_map{$src}) {
			$text_acc .= "{$img_map{$src}}";
		}
		if ($src =~ /iceborne/) {
			$is_iceborne = 1;
		}
	}
}

sub end {
	my ($self, $tag, $origtext) = @_;
	if (lc($tag) eq "table") {
		$in_subtable = 0;
	}
	return if $in_subtable;

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
	} elsif (lc($tag) eq "rt") {
		$text_acc .= ']';
	} elsif (lc($tag) eq "h6") {
		$weapon_type = lc($text_acc);
		$weapon_type =~ s/\s+/_/g;
		$weapon_type = "sword_and_shield" if ($weapon_type eq "sword_&amp;_shield");
	} elsif (lc($tag) eq "div") {
		my %coating_map = (
			"Close Range" => "coating_close_range",
			"Power" => "coating_power",
			"Paralysis" => "coating_paralysis",
			"Poison" => "coating_poison",
			"Sleep" => "coating_sleep",
			"Blast" => "coating_blast"
		);
		if ($coating_map{$text_acc_div}) {
			push @ammos, $coating_map{$text_acc_div} unless ($in_div_class);
		}
	}
}

sub text {
	my ($self, $text) = @_;
	$text_acc .= $text;
	$text_acc_div .= $text if ($in_div);
}

my $io = IO::Handle->new();
$io->fdopen(fileno(STDOUT), "w");
$xml_writer = XML::Writer->new(OUTPUT => $io, ENCODING => 'utf-8', DATA_MODE => 1, DATA_INDENT => 4);
$xml_writer->xmlDecl("UTF-8");
$xml_writer->startTag("data");

for my $file (@ARGV) {
	if ($file =~ /\/(\w\w)\//) {
		$name_key = "name_$1";
	} else {
		$name_key = "name";
	}
	my $p = HTML::Parser->new(api_version => 3,
	                          start_h => [ \&start, "self, tagname, attr, attrseq, text" ],
	                          end_h => [ \&end, "self, tagname, text" ],
	                          text_h => [ \&text, "self, text, is_cdata" ]);
	$p->parse_file($file);
}

$xml_writer->endTag();
$xml_writer->end();
# print "$xml_writer\n";
