#!/usr/bin/perl

use strict;
use warnings;

use utf8;
use open ':std', ':encoding(UTF-8)';

use XML::Writer;
use HTML::Parser;
use HTML::Entities;
use Data::Dumper;

my $xml_writer;

my $inflate_factor = 1.0;
my $extra_column = 0;
my $is_final = 0;

my @data_row = ();
my $weapon_type = "";
my %sharpness = ();
my %sharpness_plus = ();
my $fill_sharpness_plus = 0;
my @ammos = ();
my @notes = ();
my @slots = ();
my %bowgun_ammos = ();
my $is_iceborne = 0;

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

	my $has_ammos = 0;
	for my $k ("normal1", "normal2", "normal3",
	           "pierce1", "pierce2", "pierce3",
	           "spread1", "spread2", "spread3",
	           "sticky1", "sticky2", "sticky3",
	           "cluster1", "cluster2", "cluster3",
	           "recover1", "recover2", "poison1", "poison2",
	           "paralysis1", "paralysis2", "sleep1", "sleep2",
	           "exhaust1", "exhaust2",
	           "flaming", "water", "freeze", "thunder", "dragon",
	           "slicing", "wyvern", "demon", "armor", "tranq") {
		if ($bowgun_ammos{$k}) {
			unless ($has_ammos) {
				$xml_writer->startTag("ammos");
				$has_ammos = 1;
			}
			$xml_writer->startTag("ammo_ref", "id" => "ammo_$k");
			$xml_writer->dataElement("capacity", $bowgun_ammos{$k});
			$xml_writer->endTag();
		}
	}
	$xml_writer->endTag() if ($has_ammos);

	my $has_slots = 0;
	while ($data_row[2] =~ m/([①②③④])/g) {
		my $slot_level = 0;
		if ($1 eq "①") {
			$slot_level = 1;
		} elsif ($1 eq "②") {
			$slot_level = 2;
		} elsif ($1 eq "③") {
			$slot_level = 3;
		} elsif ($1 eq "④") {
			$slot_level = 4;
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
			$xml_writer->dataElement("elderseal", "average");
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
				"滅龍" => "dragon",
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

	my $has_ammos = 0;
	for my $a (@ammos) {
		unless ($has_ammos) {
			$xml_writer->startTag("ammos");
			$has_ammos = 1;
		}
		$xml_writer->emptyTag("ammo_ref", "id" => $a);
	}
	if ($extra_column) {
		if ($data_row[4] =~ /(通常|拡散|放射)\s*(\d+)/g) {
			my %shell_map = (
				"通常" => "normal",
				"拡散" => "wide",
				"放射" => "long"
			);
			if ($shell_map{$1}) {
				unless ($has_ammos) {
					$xml_writer->startTag("ammos");
					$has_ammos = 1;
				}
				$xml_writer->emptyTag("ammo_ref", "id" => "shell_${shell_map{$1}}_$2");
			}
		}
	}
	$xml_writer->endTag() if ($has_ammos);

	my $has_notes = 0;
	for my $n (@notes) {
		unless ($has_notes) {
			$xml_writer->startTag("notes");
			$has_notes = 1;
		}
		$xml_writer->emptyTag($n);
	}
	$xml_writer->endTag() if ($has_notes);

	my $has_slots = 0;
	while ($data_row[5 + $extra_column] =~ m/([①②③④])/g) {
		my $slot_level = 0;
		if ($1 eq "①") {
			$slot_level = 1;
		} elsif ($1 eq "②") {
			$slot_level = 2;
		} elsif ($1 eq "③") {
			$slot_level = 3;
		} elsif ($1 eq "④") {
			$slot_level = 4;
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
	return unless $data_row[1];

	$xml_writer->startTag("weapon");
	$xml_writer->emptyTag("weapon_type_ref", "id" => $weapon_type);

	my $is_upgraded = ($data_row[1] =~ /[\x{2517}\x{2523}]/);
	my $is_kulve = ($data_row[0] =~ /マム鑑定/);
	my $is_created = 0;
	my $name = $data_row[1];
	$name =~ s/^[\x{2500}-\x{25ff}\s]*//;
	if ($name =~ /\[生産\]/) {
		$name =~ s/\s*\[生産\]\s*//;
		$is_created = 1;
	}
	$name =~ s/\s*\[■A\]\s*//; # TODO arch kulve
	$name =~ s/\s*\[■B\]\s*//; # TODO wut?
	$name =~ s/\s*\[■S\]\s*//; # TODO wut?
	$xml_writer->dataElement("name_ja", $name);

	if ($weapon_type eq "heavy_bowgun" || $weapon_type eq "light_bowgun") {
		process_data_row_bowgun();
	} else {
		process_data_row_melee();
	}

	$xml_writer->startTag("categories");
	$xml_writer->emptyTag("category_ref", "id" => "created") if ($is_created && !$is_kulve);
	$xml_writer->emptyTag("category_ref", "id" => "upgradable") unless ($is_final);
	$xml_writer->emptyTag("category_ref", "id" => "upgraded") if ($is_upgraded);
	$xml_writer->emptyTag("category_ref", "id" => "iceborne") if ($is_iceborne);
	$xml_writer->emptyTag("category_ref", "id" => "kulve") if ($is_kulve);
	$xml_writer->endTag();
	$xml_writer->endTag();
}

my $in_bullet = 0;
my $last_span_class;
my $cur_col = 0;
my $text_acc = "";
my $text_acc_span = "";
my $bowgun_ammo_type = "";

my %data_row_span = ();
my %data_row_span_next = ();

sub start {
	my ($self, $tag, $attr, $attrseq, $origtext) = @_;
	if (lc($tag) eq "table") {
		$in_bullet = 1 if ($attr->{"class"} eq "in_bullet");
	} elsif (lc($tag) eq "td" || lc($tag) eq "th") {
		$text_acc = "";
		if (!$in_bullet && lc($tag) eq "td") {
			$data_row_span{$cur_col} = ($attr->{"rowspan"} || 1);
			$is_iceborne = 1 if (defined $attr->{"style"} && $attr->{"style"} eq "background-color:#EFF3FA;");
		}
	} elsif (lc($tag) eq "tr" && !$in_bullet) {
		$cur_col = 0;
		++$cur_col while ($data_row_span{$cur_col});
		for my $i (0 .. @data_row - 1) {
			undef $data_row[$i] unless ($data_row_span{$i});
		}
		%sharpness = ();
		%sharpness_plus = ();
		$fill_sharpness_plus = 0;
		$is_final = 0;
		@ammos = ();
		@notes = ();
		@slots = ();
		%bowgun_ammos = ();
		$is_iceborne = 0;
	} elsif (lc($tag) eq "span") {
		$last_span_class = $attr->{"class"};
		$text_acc_span = "";
		if ($attr->{"style"}) {
			if ($attr->{"style"} eq "background-color:#eec3e6;") {
				$is_final = 1;
			} elsif ($cur_col == 4) {
				my %note_map = (
					"color:#f3f3f3;" => "white",
					"color:#c778c7;" => "purple",
					"color:#e0002a;" => "red",
					"color:blue;" => "blue",
					"color:#00cc00;" => "green",
					"color:#eeee00;" => "yellow",
					"color:#99f8f8;" => "light_blue",
					"color:#ef810f;" => "orange"
				);
				if ($note_map{lc($attr->{"style"})}) {
					push @notes, $note_map{lc($attr->{"style"})};
				}
			}
		}
	}
}

sub end {
	my ($self, $tag, $origtext) = @_;
	if (lc($tag) eq "table") {
		$in_bullet = 0;
	} elsif (lc($tag) eq "td" || lc($tag) eq "th") {
		my $t = $text_acc;
		utf8::decode($t);
		$t =~ s/&#(\d+);/$1 == 9495 ? "" : chr($1)/eg;
		$t = decode_entities($t);
		$t =~ s/\s+/ /g;
		$t =~ s/^\s+//;
		$t =~ s/\s+$//;
#		print "$cur_col: $t\n";
		if (!$in_bullet && lc($tag) eq "td") {
			$data_row[$cur_col] = $t;
			++$cur_col;
			++$cur_col while ($data_row_span{$cur_col});
		} elsif ($in_bullet && lc($tag) eq "th") {
			my %ammo_map = (
				"通" => "normal1",
				"回" => "recover1",
				"火" => "flaming",
				"斬" => "slicing",
				"貫" => "pierce1",
				"毒" => "poison1",
				"水" => "water",
				"竜" => "wyvern",
				"散" => "spread1",
				"麻" => "paralysis1",
				"氷" => "freeze",
				"鬼" => "demon",
				"徹" => "sticky1",
				"睡" => "sleep1",
				"雷" => "thunder",
				"硬" => "armor",
				"拡" => "cluster1",
				"減" => "exhaust1",
				"龍" => "dragon",
				"捕" => "tranq"
			);
			if ($ammo_map{$t}) {
				$bowgun_ammo_type = $ammo_map{$t};
			}
		} elsif ($in_bullet && lc($tag) eq "td") {
			if ($t =~ /\d+/) {
				$bowgun_ammos{$bowgun_ammo_type} = $t;
			}
			$bowgun_ammo_type =~ s/(\d+)/$1 + 1/e;
		}
	} elsif (lc($tag) eq "tr" && (@data_row > 0) && !$in_bullet) {
		process_data_row();
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
			} elsif ($last_span_class eq "c_g b" || $last_span_class eq "c_r b") {
				my %coating_map = (
					"接" => "coating_close_range",
					"強" => "coating_power",
					"麻" => "coating_paralysis",
					"毒" => "coating_poison",
					"睡" => "coating_sleep",
					"爆" => "coating_blast"
				);
				my $t = $text_acc_span;
				utf8::decode($t);
				if ($coating_map{$t}) {
					push @ammos, $coating_map{$t};
				}
				undef $last_span_class;
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
