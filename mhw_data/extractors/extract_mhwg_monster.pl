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

my @data_row = ();
my @header_row = ();
my $weapon_type = "";
my %sharpness = ();
my @slots = ();
my $monster_name;

my @monsters;
my $cur_monster;

my %monster_states = (
	"Anjanath" => [ "", "Wounded" ],
	"Anjanath//破壊後" => [ "Wounded" ],
	"Barroth" => [ "", "Mud" ],
	"Dodogama" => [ "", "Rock 1", "Rock 2" ],
	"Dodogama//石溜め1" => [ "Rock 1" ],
	"Dodogama//石溜め2" => [ "Rock 2" ],
	"Great Jagras/腹膨張" => [ "Inflated" ],
	"Jyuratodus" => [ "", "Mud" ],
	"Kirin" => [ "", "Electricity" ],
	"Lavasioth/頭" => [ "Magma Armor", "Wounded", "Heated" ],
	"Lavasioth/胴" => [ "", "Heated" ],
	"Lavasioth/背中" => [ "Magma Armor", "Wounded", "Heated" ],
	"Lavasioth/脚" => [ "Magma Armor", "Wounded", "Heated" ],
	"Lavasioth/尻尾" => [ "Magma Armor", "Wounded", "Heated" ],
	"Nergigante/角" => [ "", "Wounded" ],
	"Nergigante" => [ "", "White", "Black" ],
	"Uragaan" => [ "", "Wounded" ],
	"Uragaan//破壊後" => [ "Wounded" ],
	"Vaal Hazak" => [ "", "Wounded" ],
	"Xeno'jiiva" => [ "", "Critical State", "Wounded" ],
	"Zorah Magdaros/胸" => [ "Before Wounded", "After Wounded" ],
	"Deviljho" => [ "", "Enraged" ],
	"Kulve Taroth/頭" => [ "Start", "Wounded", "" ],
	"Kulve Taroth/胸" => [ "Start", "" ],
	"Kulve Taroth/前脚" => [ "Start", "" ],
	"Behemoth" => [ "", "Wounded" ],
	"Glavenus//赤熱" => [ "Heated" ],
	"Glavenus//サビ" => [ "Rust" ],
	"Nargacuga//破壊後" => [ "Wounded" ],
	"Banbaro//破壊後" => [ "Wounded" ],
	"Beotodus//雪纏い" => [ "Snow" ],
	"Barioth//破壊後" => [ "Wounded" ],
	"Ebony Odogaron//破壊後" => [ "Wounded" ],
	"Acidic Glavenus//結晶" => [ "Crystalized" ],
	"Acidic Glavenus//研ぎ" => [ "Sharpened" ],
	"Velkhana" => [ "", "Broken" ],
	"Fulgur Anjanath" => [ "", "Charged" ],
	"Fulgur Anjanath/脚" => [ "", "Broken", "Charged" ],
	"Fulgur Anjanath/尻尾" => [ "", "Broken", "Charged" ],
	"Namielle" => [ "", "Water" ],
	"Yian Garuga" => [ "", "Broken" ],
	"Scarred Yian Garuga" => [ "", "Broken" ],
	"Zinogre" => [ "", "Charged" ],
	"Rajang" => [ "", "Enraged" ],
	"Ruiner Nergigante/角" => [ "", "Wounded" ],
	"Ruiner Nergigante" => [ "", "Regeneration", "Super Regeneration" ],
	"Silver Rathalos" => [ "", "Broken" ],
	"Gold Rathian" => [ "", "Broken" ],
	"Savage Deviljho" => [ "", "Enraged" ],
	"Blackveil Vaal Hazak" => [ "", "Wounded" ]
);

my %monster_enraged_states = (
	"Deviljho" => [ 0, 1 ],
	"Savage Deviljho" => [ 0, 1 ],
	"Rajang" => [ 0, 1 ]
);

sub process_data_row {
	my @drow = @data_row;
	my %lrow = ();
	for (my $i = 0; $i < @drow; ++$i) {
		if (defined $header_row[$i] && defined $data_row[$i]) {
			$lrow{$header_row[$i]} = $data_row[$i];
		}
	}

	my @lrow_s = ();
	for my $key (keys %lrow) {
		my $v = $lrow{$key};
		$v =~ s/[()]|^\s*|\s*$//g;
		my $idx = 0;
		for my $vs (split /\s+/, $v) {
			$lrow_s[$idx] ||= {};
			$lrow_s[$idx]->{$key} = $vs;
			++$idx;
		}
	}

	for (my $i = 1; $i < @lrow_s; ++$i) {
		for my $key (keys %{$lrow_s[$i - 1]}) {
			unless (defined $lrow_s[$i]->{$key}) {
				$lrow_s[$i]->{$key} = $lrow_s[$i - 1]->{$key};
			}
		}
	}

	for my $part_name (split /・/, $header_row[0]) {
		my $state_name = "";
		if ($part_name =~ /^\s*(.*)\s*\((.*)\)\s*$/) {
			$part_name = $1;
			$state_name = $2;
		}

		unless ($cur_monster->{"hit_data"}{$part_name}) {
			push @{$cur_monster->{"parts"}}, $part_name;
			$cur_monster->{"hit_data"}{$part_name} = { };
			$cur_monster->{"last_state_id"}{$part_name} ||= {};
			$cur_monster->{"last_state_id"}{$part_name}{$state_name} = 0;
		}

		for my $row_s (@lrow_s) {
			my $state_id = $cur_monster->{"last_state_id"}{$part_name}{$state_name}++;
			my $state = $state_id == 0 ? "$state_name" : "state$state_name$state_id";
			my $n = $cur_monster->{"name"} || $cur_monster->{"name_ja"};
			my @xns = ($n . "/" . $part_name . "/" . $state_name,
			           $n . "//" . $state_name, $n . "/" . $part_name, $n);
			for my $xn (@xns) {
				if ($monster_states{$xn}) {
					if (defined $monster_states{$xn}->[$state_id]) {
						$state = $monster_states{$xn}->[$state_id];
					}
					last;
				}
			}
			$cur_monster->{"hit_data"}{$part_name}{$state} = {
				"cut" => $row_s->{"切"},
				"impact" => $row_s->{"打"},
				"bullet" => $row_s->{"弾"},
				"fire" => $row_s->{"火"},
				"water" => $row_s->{"水"},
				"thunder" => $row_s->{"雷"},
				"ice" => $row_s->{"氷"},
				"dragon" => $row_s->{"龍"},
				"stun" => $row_s->{"気絶"}
			};

			my $enraged_state;
			for my $xn (@xns) {
				if ($monster_enraged_states{$xn}) {
					if (defined $monster_enraged_states{$xn}->[$state_id]) {
						$enraged_state = $monster_enraged_states{$xn}->[$state_id];
					}
				}
			}
			$cur_monster->{"hit_data"}{$part_name}{$state}{"enraged_state"} = $enraged_state;
		}
	}
}

sub process_status_row {
	my %xname = (
		"毒" => "poison",
		"麻痺" => "paralysis",
		"睡眠" => "sleep",
		"気絶" => "stun",
		"爆破" => "blast",
		"減気" => "exhaust",
		"乗り" => "mount"
	);
	my %xinit = (
		"◎" => 75,
		"○" => 150,
		"◯" => 150,
		"△" => 300,
		"×" => 450
	);
	my %xplus = (
		"◎" => 50,
		"○" => 100,
		"◯" => 100,
		"△" => 200,
		"×" => 300
	);
	my %xduration = (
		"◎" => 60,
		"○" => 30,
		"◯" => 30,
		"△" => 15,
		"×" => 0
	);
	my $sta_name = $xname{$header_row[0]};
	my $sta_init = ($data_row[1] && $xinit{$data_row[1]}) ? $xinit{$data_row[1]} : 0;
	my $sta_plus = ($data_row[1] && $xplus{$data_row[1]}) ? $xplus{$data_row[1]} : 0;
	my $sta_duration = ($data_row[2] && $xduration{$data_row[2]}) ? $xduration{$data_row[2]} : 0;
	if ($sta_init) {
		$cur_monster->{"tolerances"}{$sta_name} = {
			"initial" => $sta_init,
			"plus" => $sta_plus,
			"max" => $sta_init + $sta_plus * 4,
		};
		if ($sta_name ne "blast") {
			$cur_monster->{"tolerances"}{$sta_name}{"regen_value"} = 5;
			$cur_monster->{"tolerances"}{$sta_name}{"regen_tick"} = 10;
		}
		if ($sta_name eq "blast") {
			$cur_monster->{"tolerances"}{$sta_name}{"damage"} = 120;
		}
		if ($sta_name eq "poison") {
			$cur_monster->{"tolerances"}{$sta_name}{"duration"} = $sta_duration;
			$cur_monster->{"tolerances"}{$sta_name}{"damage"} = $sta_duration * 4;
		}
	}
}

my $in_hit_data = 0;
my $in_tr = 0;
my $in_td = 0;
my $last_th = "";
my $cur_col = 0;
my @text_stack = ();
my %important_tags = map { $_ => 1 } ("div", "td", "tr", "h1", "h4", "th", "table");

sub decode_text {
	my $t = $_[0];
	$t =~ s/&#(\d+);/$1 == 9495 ? "" : chr($1)/eg;
	$t = decode_entities($t);
	$t =~ s/\s+/ /g;
	$t =~ s/^\s+//;
	$t =~ s/\s+$//;
	return $t;
}

sub start {
	my ($self, $tag, $attr, $attrseq, $origtext) = @_;

	if (lc($tag) eq "br") {
		for my $i (0 .. (@text_stack - 1)) {
			$text_stack[$i] .= "\n";
		}
	} elsif (lc($tag) eq "img") {
		if (defined $attr->{"alt"}) {
			my $text = $attr->{"alt"};
			utf8::decode($text);
			for my $i (0 .. (@text_stack - 1)) {
				$text_stack[$i] .= $text;
			}
		}
	}

	return unless $important_tags{$tag};
	push @text_stack, "";
	if (lc($tag) eq "div") {
		++$in_hit_data if ($in_hit_data > 0);
	} elsif (lc($tag) eq "table") {
		@header_row = ();
	} elsif (lc($tag) eq "td") {
		$in_td = 1;
	} elsif (lc($tag) eq "tr") {
		$in_tr = 0;
		$in_td = 0;
		$cur_col = 0;
		@data_row = ();
	}
}

sub end {
	my ($self, $tag, $origtext) = @_;
	return unless $important_tags{$tag};
	my $text = pop @text_stack;
	if (lc($tag) eq "div") {
		--$in_hit_data if ($in_hit_data > 0);
		if ($text =~ /^\s*([,\d]+)\s*Base HP\s*$/) {
			my $hp = $1;
			$hp =~ s/,//g;
			$cur_monster->{"hit_points"} = $hp;
		}
	} elsif (lc($tag) eq "td") {
		$in_td = 0;
		$data_row[$cur_col] = decode_text($text);
#		print "td: $text\n";
#		print "$cur_col: $t\n";
		if ($last_th eq "英語名") {
			my $t = decode_text($text);
			$t =~ s/([a-z])([A-Z])/$1 $2/g;
			$t = "Xeno'jiiva" if ($t eq "Xenojiiva");
			$cur_monster->{"name"} = $t if ($t =~ /\w/);
		} elsif ($last_th eq "モンスター名") {
			$cur_monster->{"name_ja"} = decode_text($text);
		}
		++$cur_col;
	} elsif (lc($tag) eq "tr" && (@data_row > 0)) {
#		print Dumper(\@header_row);
		process_data_row() if ($header_row[9]);
		process_status_row() if ($header_row[2] && $header_row[2] eq "ダメージ・効果時間");
		$in_tr = 0;
	} elsif (lc($tag) eq "h4") {
		if ($text =~ /\s*(.*\S)\s+hit data/) {
			$in_hit_data = 1;
		}
	} elsif (lc($tag) eq "th") {
		$last_th = $text;
		$header_row[$cur_col] = $text;
		++$cur_col;
	} elsif (lc($tag) eq "h1") {
		$cur_monster = {
			"hit_data" => {},
			"parts" => [],
			"last_state_id" => {}
		};
		push @monsters, $cur_monster;
	}
}

sub text {
	my ($self, $text) = @_;
	utf8::decode($text);
	for my $i (0 .. (@text_stack - 1)) {
		$text_stack[$i] .= $text;
	}
}

my $io = IO::Handle->new();
$io->fdopen(fileno(STDOUT), "w");
$xml_writer = XML::Writer->new(OUTPUT => $io, ENCODING => 'utf-8', DATA_MODE => 1, DATA_INDENT => 4);
$xml_writer->xmlDecl("UTF-8");
$xml_writer->startTag("data");

for my $file (@ARGV) {
	my $p = HTML::Parser->new(api_version => 3,
	                          start_h => [ \&start, "self, tagname, attr, attrseq, text" ],
	                          end_h => [ \&end, "self, tagname, text" ],
	                          text_h => [ \&text, "self, text, is_cdata" ]);
	$p->parse_file($file);
}

for my $monster (@monsters) {
	$xml_writer->startTag("monster");
	$xml_writer->dataElement("name", $monster->{"name"}) if ($monster->{"name"});
	$xml_writer->dataElement("name_ja", $monster->{"name_ja"}) if ($monster->{"name_ja"});
	$xml_writer->dataElement("hit_points", $monster->{"hit_points"}) if ($monster->{"hit_points"});
	for my $part_name (@{$monster->{"parts"}}) {
		$xml_writer->startTag("part");
		$xml_writer->dataElement("name_ja", $part_name);
		for my $state (sort keys %{$monster->{"hit_data"}{$part_name}}) {
			$xml_writer->startTag("hit_data");
			$xml_writer->dataElement("state", $state) if ($state);
			if (defined $monster->{"hit_data"}{$part_name}{$state}{"enraged_state"}) {
				$xml_writer->dataElement("enraged_state",
				                         $monster->{"hit_data"}{$part_name}{$state}{"enraged_state"});
			}
			for my $key ("cut", "impact", "bullet", "fire", "water", "thunder", "ice", "dragon", "stun") {
				$xml_writer->dataElement($key, $monster->{"hit_data"}{$part_name}{$state}{$key});
			}
			$xml_writer->endTag();
		}
		$xml_writer->endTag();
	}
	for my $tol_type ("poison", "paralysis", "sleep", "stun", "blast", "exhaust", "mount") {
		next unless ($monster->{"tolerances"}{$tol_type});
		$xml_writer->startTag("tolerance");
		$xml_writer->dataElement("type", $tol_type);
		for my $tol_data ("initial", "plus", "max", "regen_value", "regen_tick", "duration", "damage") {
			next unless (defined $monster->{"tolerances"}{$tol_type}{$tol_data});
			$xml_writer->dataElement($tol_data, $monster->{"tolerances"}{$tol_type}{$tol_data});
		}
		$xml_writer->endTag();
	}
	$xml_writer->endTag();
}

$xml_writer->endTag();
$xml_writer->end();
# print "$xml_writer\n";
