#!/usr/bin/perl

use strict;
use warnings;

use utf8;
use open ':std', ':encoding(UTF-8)';

use XML::Writer;
use HTML::Parser;
use HTML::Entities;

my $xml_writer;

my $inflate_factor = 1.0;
my $extra_column = 0;
my $is_final = 0;

my @data_row = ();
my $weapon_type = "";
my %sharpness = ();
my %sharpness_plus = ();
my $fill_sharpness_plus = 0;
my @slots = ();

sub process_data_row_bowgun {
	if ($data_row[2] =~ /攻撃[：:\s]*(\d+)/) {
		$xml_writer->dataElement("inflated_attack", $1);
		$xml_writer->dataElement("attack", $1 / $inflate_factor);
	}

	if ($data_row[2] =~ /会心\s*(-?\d+)/) {
		$xml_writer->dataElement("affinity", $1);
	}

	if ($data_row[2] =~ /防御\s*\+?(-?\d+)/) {
		$xml_writer->dataElement("defense", $1);
	}

	my $has_slots = 0;
	while ($data_row[2] =~ m/([①②③])/g) {
		my $slot_level = 0;
		if ($1 eq "①") {
			$slot_level = 1;
		} elsif ($1 eq "②") {
			$slot_level = 2;
		} elsif ($1 eq "③") {
			$slot_level = 3;
		}
		unless ($has_slots) {
			$xml_writer->startTag("slots");
			$has_slots = 1;
		}
		$xml_writer->dataElement("slot", $slot_level);
	}
	$xml_writer->endTag() if ($has_slots);
}

sub process_data_row_melee {
	$xml_writer->dataElement("inflated_attack", $data_row[2]);
	$xml_writer->dataElement("attack", $data_row[2] / $inflate_factor);
	if ($data_row[3] =~ /会心\s*(-?\d+)/) {
		$xml_writer->dataElement("affinity", $1);
	}

	if ($data_row[3] =~ /\(/) {
		$xml_writer->dataElement("awakened", "true");
	}

	my $has_element = 0;
	while ($data_row[3] =~ /([^(\s\d]+)\s*(\d+)/g) {
		my %elt_map = (
			"水" => "water",
			"火" => "fire",
			"雷" => "thunder",
			"氷" => "ice",
			"龍" => "dragon",
			"爆破" => "blast",
			"睡眠" => "sleep",
			"麻痺" => "paralysis",
			"毒" => "poison",
			"減気" => "exhaust"
		);
		if ($elt_map{$1}) {
			unless ($has_element) {
				$xml_writer->startTag("element");
				$has_element = 1;
			}
			$xml_writer->dataElement("inflated_" . $elt_map{$1}, $2);
			$xml_writer->dataElement($elt_map{$1}, $2 / 10);
		}
	}
	$xml_writer->endTag() if ($has_element);

	if ($data_row[3] =~ /龍封力\s*\[([^\]]*)\]/) {
		if ($1 eq "小") {
			$xml_writer->dataElement("elderseal", "low");
		} elsif ($1 eq "中") {
			$xml_writer->dataElement("elderseal", "medium");
		} elsif ($1 eq "大") {
			$xml_writer->dataElement("elderseal", "high");
		}
	}

	if ($data_row[3] =~ /防御\s*\+?(-?\d+)/) {
		$xml_writer->dataElement("defense", $1);
	}

	if ($extra_column) {
		my $has_phial = 0;
		while ($data_row[4] =~ /([^(\s\d]+)\s*(\d+)/g) {
			my %elt_map = (
				"水" => "water",
				"火" => "fire",
				"雷" => "thunder",
				"氷" => "ice",
				"龍" => "dragon",
				"爆破" => "blast",
				"睡眠" => "sleep",
				"麻痺" => "paralysis",
				"毒" => "poison",
				"減気" => "exhaust"
			);
			if ($elt_map{$1}) {
				unless ($has_phial) {
					$xml_writer->startTag("phial");
					$has_phial = 1;
				}
				$xml_writer->dataElement("inflated_" . $elt_map{$1}, $2);
				$xml_writer->dataElement($elt_map{$1}, $2 / 10);
			}
		}
		if ($data_row[4] =~ /(強撃|強属性|榴弾)/) {
			unless ($has_phial) {
				$xml_writer->startTag("phial");
				$has_phial = 1;
			}
			if ($1 eq "強撃") {
				$xml_writer->emptyTag("power");
			} elsif ($1 eq "強属性") {
				$xml_writer->emptyTag("element");
			} elsif ($1 eq "榴弾") {
				$xml_writer->emptyTag("impact");
			}
		}
		$xml_writer->endTag() if ($has_phial);
	}


	my $has_sharpness = 0;
	my $sharpness_plus_val = 0;
	for my $sharpness_color ("red", "orange", "yellow", "green", "blue", "white", "purple") {
		if ($sharpness{$sharpness_color} || $sharpness_plus{$sharpness_color}) {
			unless ($has_sharpness) {
				$xml_writer->startTag("sharpness");
				$has_sharpness = 1;
			}
			if ($sharpness_plus{$sharpness_color}) {
				$sharpness_plus_val += ($sharpness_plus{$sharpness_color} -
				                        ($sharpness{$sharpness_color} || 0)) * 10;
				$xml_writer->dataElement($sharpness_color, $sharpness_plus{$sharpness_color} * 10);
			} else {
				$xml_writer->dataElement($sharpness_color, $sharpness{$sharpness_color} * 10);
			}
		}
	}
	$xml_writer->dataElement("plus", $sharpness_plus_val) if ($sharpness_plus_val);
	$xml_writer->endTag() if ($has_sharpness);

	my $has_slots = 0;
	while ($data_row[5 + $extra_column] =~ m/([①②③])/g) {
		my $slot_level = 0;
		if ($1 eq "①") {
			$slot_level = 1;
		} elsif ($1 eq "②") {
			$slot_level = 2;
		} elsif ($1 eq "③") {
			$slot_level = 3;
		}
		unless ($has_slots) {
			$xml_writer->startTag("slots");
			$has_slots = 1;
		}
		$xml_writer->dataElement("slot", $slot_level);
	}
	$xml_writer->endTag() if ($has_slots);
}


sub process_data_row {
	return unless defined $data_row[1];

	$xml_writer->startTag("weapon");
	$xml_writer->dataElement("type", $weapon_type);

	my $creatable = 0;
	my $name = $data_row[1];
	$name =~ s/^[\x{2500}-\x{25ff}\s]*//;
	if ($name =~ /\[生産\]/) {
		$name =~ s/\s*\[生産\]\s*//;
		$creatable = 1;
	}
	$xml_writer->dataElement("name_jp", $name);

	if ($weapon_type eq "heavy_bowgun" || $weapon_type eq "light_bowgun") {
		process_data_row_bowgun();
	} else {
		process_data_row_melee();
	}

	$xml_writer->dataElement("creatable", "true") if ($creatable);
	$xml_writer->dataElement("final", "true") if ($is_final);
	$xml_writer->endTag();
}

my $in_tr = 0;
my $in_td = 0;
my $in_bullet = 0;
my $last_span_class;
my $cur_col = 0;
my $text_acc = "";
my $text_acc_span = "";

my %data_row_span = ();
my %data_row_span_next = ();

sub start {
	my ($self, $tag, $attr, $attrseq, $origtext) = @_;
	if (lc($tag) eq "table") {
		$in_bullet = 1 if ($attr->{"class"} eq "in_bullet");
	} elsif (lc($tag) eq "td" && !$in_bullet) {
		$in_td = 1;
		$text_acc = "";
		$data_row_span{$cur_col} = ($attr->{"rowspan"} || 1);
	} elsif (lc($tag) eq "tr" && !$in_bullet) {
		$in_tr = 0;
		$in_td = 0;
		$cur_col = 0;
		++$cur_col while ($data_row_span{$cur_col});
		@data_row = ();
		%sharpness = ();
		%sharpness_plus = ();
		$fill_sharpness_plus = 0;
		$is_final = 0;
		@slots = ();
	} elsif (lc($tag) eq "span") {
		$last_span_class = $attr->{"class"};
		$text_acc_span = "";
		if ($attr->{"style"} && $attr->{"style"} eq "background-color:#eec3e6;") {
			$is_final = 1;
		}
	}
}

sub end {
	my ($self, $tag, $origtext) = @_;
	if (lc($tag) eq "table") {
		$in_bullet = 0;
	} elsif (lc($tag) eq "td" && !$in_bullet) {
		$in_td = 0;
		my $t = $text_acc;
		utf8::decode($t);
		$t =~ s/&#(\d+);/$1 == 9495 ? "" : chr($1)/eg;
		$t = decode_entities($t);
		$t =~ s/\s+/ /g;
		$t =~ s/^\s+//;
		$t =~ s/\s+$//;
		# print "$cur_col: $t\n";
		$data_row[$cur_col] = $t;
		++$cur_col;
		++$cur_col while ($data_row_span{$cur_col});
	} elsif (lc($tag) eq "tr" && (@data_row > 0) && !$in_bullet) {
		process_data_row();
		$in_tr = 0;
		for  my $k (keys %data_row_span) {
			if ($data_row_span{$k} > 1) {
				--$data_row_span{$k};
			} else {
				delete $data_row_span{$k};
			}
		}
	} elsif (lc($tag) eq "span") {
		if (defined $last_span_class) {
			if ($last_span_class =~ /kr(\d)/) {
				my %color = (0 => "red", 1 => "orange", 2 => "yellow",
				             3 => "green", 4 => "blue", 5 => "white",
				             6 => "purple", 7 => "black");
				$fill_sharpness_plus = 1 if ($1 == 0 && %sharpness);
				if ($fill_sharpness_plus) {
					$sharpness_plus{$color{$1}} = length($text_acc_span);
				} else {
					$sharpness{$color{$1}} = length($text_acc_span);
				}
			}
		}
	}
}

sub text {
	my ($self, $text) = @_;
	$text_acc .= $text;
	$text_acc_span .= $text;
}

my $io = IO::Handle->new();
$io->fdopen(fileno(STDOUT), ">");
$xml_writer = XML::Writer->new(OUTPUT => $io, ENCODING => 'utf-8', DATA_MODE => 1, DATA_INDENT => 4);
$xml_writer->xmlDecl("UTF-8");
$xml_writer->startTag("data");

for my $file (@ARGV) {
	$weapon_type = "no_type";
	$inflate_factor = 1.0;
	if ($file =~ /.*\/(\d+)\.html/) {
		my %idn = (
			4000 => "great_sword",
			4001 => "long_sword",
			4002 => "sword_and_shield",
			4003 => "dual_blades",
			4004 => "hammer",
			4005 => "hunting_horn",
			4006 => "lance",
			4007 => "gunlance",
			4008 => "switch_axe",
			4009 => "charge_blade",
			4010 => "insect_glaive",
			4011 => "light_bowgun",
			4012 => "heavy_bowgun",
			4013 => "bow"
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
		my %extra_col_by_type = (
			"great_sword" => 0,
			"long_sword" => 0,
			"sword_and_shield" => 0,
			"dual_blades" => 0,
			"hammer" => 0,
			"hunting_horn" => 1,
			"lance" => 0,
			"gunlance" => 1,
			"switch_axe" => 1,
			"charge_blade" => 1,
			"insect_glaive" => 1,
			"light_bowgun" => 0,
			"heavy_bowgun" => 0,
			"bow" => 0
		);
		$weapon_type = $idn{$1};
		$inflate_factor = $inflate_by_type{$weapon_type};
		$extra_column = $extra_col_by_type{$weapon_type};
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