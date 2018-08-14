#!/usr/bin/perl

use strict;
use warnings;

use XML::Writer;
use HTML::Parser;
use HTML::Entities;
use Data::Dumper;

my $xml_writer;

my @data_row = ();
my @header_row = ();
my $weapon_type;

my @motion_values;

sub get_totals {
	my $mv_string = $_[0];
	my $sum = 0;
	my $hits = 0;

	if ($mv_string =~ /^[\s-]*$/) {
		return (0, 0);
	}
	# blunt kinsect only (need fix)
	$mv_string =~ s/\*$//;
	for my $mv (split /\s*\+\s*/, $mv_string) {
		my $rep = 1;
		if ($mv =~ /^(.*)x\s*(\d+)\s*$/) {
			$mv = $1;
			$rep = $2;
		} elsif ($mv =~ /^(.*)x\s*N\s*$/) {
			# for the hammer air attack, say N=4 for no good reason
			$mv = $1;
			$rep = 4;
		}
		$sum += $mv * $rep;
		$hits += $rep;
	}
	return ($sum, $hits);
}

my %weapon_id_prefix = (
	"Bow" => "bow",
	"Charge Blade" => "cb",
	"Dual Blades" => "db",
	"Great Sword" => "gs",
	"Gunlance" => "gl",
	"Hammer" => "hammer",
	"Heavy Bowgun" => "hbg",
	"Hunting Horn" => "hh",
	"Insect Glaive" => "ig",
	"Lance" => "lance",
	"Light Bowgun" => "lbg",
	"Long Sword" => "ls",
	"Switch Axe" => "sa",
	"Sword & Shield" => "sns"
);

my %weapon_type_id = (
	"Bow" => "bow",
	"Charge Blade" => "charge_blade",
	"Dual Blades" => "dual_blades",
	"Great Sword" => "great_sword",
	"Gunlance" => "gunlance",
	"Hammer" => "hammer",
	"Heavy Bowgun" => "heavy_bowgun",
	"Hunting Horn" => "hunting_horn",
	"Insect Glaive" => "insect_glaive",
	"Lance" => "lance",
	"Light Bowgun" => "light_bowgun",
	"Long Sword" => "long_sword",
	"Switch Axe" => "switch_axe",
	"Sword & Shield" => "sword_and_Shield"
);

sub process_data_row {
	my @drow = @data_row;
	for (my $i = 1; $i < @drow; ++$i) { $drow[$i] =~ s/^~//; }
	my %lrow = ();
	for (my $i = 0; $i < @drow; ++$i) {
		$lrow{$header_row[$i]} = $data_row[$i];
	}

	my $mv_string = $lrow{"Motion Value"};
	my $name = $lrow{"Move"};

	if ($mv_string =~ /^(.*), up to (\d+) hits\s*$/) {
		$mv_string = $1;
		$name .= ", per hit";
	} elsif ($mv_string =~ /^(.*)\s*\(multiple times\)$/) {
		$mv_string = $1;
		$name .= ", per hit";
	} elsif ($mv_string =~ /^(.*)\s*\(per Wyvernsnipe Hit\)$/) {
		$mv_string = $1;
		$name .= ", per hit";
	}

	my ($mv, $hits) = get_totals($mv_string);
	my ($stun, $stun_hits) = get_totals($lrow{"Stun"});
	my ($exhaust, $exhaust_hits) = get_totals($lrow{"Exhaust"});

	if ($lrow{"Hit"}) {
		if  ($lrow{"Hit"} eq "N") {
			$name .= ", per hit";
		} else {
			$mv *= $lrow{"Hit"};
			$hits *= $lrow{"Hit"};
		}
	}

	my $id = lc($name);
	$id =~ s/[^a-z0-9]+/_/g;
	$id =~ s/~_|_$//g;
	$id =~ s/_i_ii_iii$//;
	$id =~ s/_i$/_1/;
	$id =~ s/_ii$/_2/;
	$id =~ s/_iii$/_3/;
	$id =~ s/_iv$/_4/;
	$id =~ s/_per_hit$/_hit/;
	$id = $weapon_id_prefix{$weapon_type} . "_$id";
	$name = "$weapon_type: " . $name;

	my $cut = 0;
	my $impact = 0;
	my $piercing = 0;
	my $bullet = 0;
	my $dtype_string = $lrow{"Damage Type"};
	if ($dtype_string eq "Sever") {
		if ($weapon_type eq "Lance") {
			$piercing = $mv;
		} else {
			$cut = $mv;
		}
	} elsif ($dtype_string eq "Blunt") {
		$impact = $mv;
	} elsif ($dtype_string eq "Sever + Blunt") {
		# SnS stuff
		if ($mv_string =~ /^\s*(\d+)\s*\+\s*(\d+)\s*$/) {
			$cut = $2;
			$impact = $1;
		} else {
			$cut = $mv;
			$impact = 0;
		}
	} else {
		$bullet = $mv;
	}

	#printf "%s / %s: c=%g, i=%g, b=%g, h=%g, s=%g, e=%g\n", $lrow{"Damage Type"}, $name, $cut, $impact, $bullet, $hits, $stun, $exhaust;

	my $mv_data = {
		"name" => $name,
		"id" => $id,
		"weapon_type_ref" => $weapon_type_id{$weapon_type}
	};
	$mv_data->{"cut"} = $cut / 100 if ($cut);
	$mv_data->{"impact"} = $impact / 100 if ($impact);
	$mv_data->{"piercing"} = $piercing / 100 if ($piercing);
	$mv_data->{"bullet"} = $bullet / 100 if ($bullet);
	$mv_data->{"element"} = $hits if ($hits);
	$mv_data->{"stun"} = $stun if ($stun);
	$mv_data->{"exhaust"} = $exhaust if ($exhaust);
	my $sharp_mod = 1.0;
	$sharp_mod = 0.35 if ($weapon_type eq "Dual Blades");
	$mv_data->{"sharpness_use"} = $hits * $sharp_mod if ($hits);
	push @motion_values, $mv_data;
}

my $in_del = 0;
my $in_motion_values = 0;
my @text_stack = ();
my %important_tags = map { $_ => 1 } ("td", "tr", "h1", "h2", "th", "thead", "table", "del");

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
	return unless $important_tags{$tag};
	push @text_stack, "";
	if (lc($tag) eq "table") {
		++$in_motion_values if ($in_motion_values > 0);
	} elsif (lc($tag) eq "thead") {
		@header_row = ();
	} elsif (lc($tag) eq "tr") {
		@data_row = ();
	} elsif (lc($tag) eq "del") {
		++$in_del;
	}
}

sub end {
	my ($self, $tag, $origtext) = @_;
	return unless $important_tags{$tag};
	my $text = pop @text_stack;
	if (lc($tag) eq "table") {
		--$in_motion_values if ($in_motion_values > 0);
	} elsif (lc($tag) eq "td") {
		push @data_row, decode_text($text);
	} elsif (lc($tag) eq "tr" && (@data_row > 0)) {
		process_data_row() if ($in_motion_values > 0 && $header_row[4]);
	} elsif (lc($tag) eq "h2") {
		if ($text =~ /Motion Values/) {
			$in_motion_values = 1;
		}
	} elsif (lc($tag) eq "th") {
		push @header_row, $text;
	} elsif (lc($tag) eq "h1") {
		$weapon_type = decode_text($text);
	} elsif (lc($tag) eq "del") {
		--$in_del;
	}
}

sub text {
	my ($self, $text) = @_;
	unless ($in_del) {
		for my $i (0 .. (@text_stack - 1)) {
			$text_stack[$i] .= $text;
		}
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

for my $mv (@motion_values) {
	$xml_writer->startTag("motion_value", "id" => $mv->{"id"});
	for my $k ("weapon_type_ref", "name", "cut", "impact", "piercing", "bullet",
	           "element", "stun", "exhaust", "sharpness_use") {
		if (defined $mv->{$k}) {
			if ($k eq  "weapon_type_ref") {
				$xml_writer->emptyTag($k, "id" => $mv->{$k})
			} else {
				$xml_writer->dataElement($k, $mv->{$k})
			}
		}
	}
	$xml_writer->endTag();
}

$xml_writer->endTag();
$xml_writer->end();
# print "$xml_writer\n";
