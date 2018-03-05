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
my $weapon_type = "";
my %sharpness = ();
my @slots = ();

sub process_data_row {
	my @drow = @data_row;
	for (my $i = 1; $i < @drow; ++$i) { $drow[$i] =~ s/^~//; }
	my %lrow = ();
	for (my $i = 0; $i < @drow; ++$i) {
		$lrow{$header_row[$i]} = $data_row[$i];
	}
	$xml_writer->startTag("part");
	$xml_writer->dataElement("name", $lrow{"Body Part"});
	$xml_writer->startTag("hit_data");
	$xml_writer->dataElement("cut", $lrow{"Sever"});
	$xml_writer->dataElement("impact", $lrow{"Blunt"});
	$xml_writer->dataElement("bullet", $lrow{"Shot"});
	$xml_writer->dataElement("fire", $lrow{"Fir"});
	$xml_writer->dataElement("water", $lrow{"Wat"});
	$xml_writer->dataElement("thunder", $lrow{"Thn"});
	$xml_writer->dataElement("ice", $lrow{"Ice"});
	$xml_writer->dataElement("dragon", $lrow{"Drg"});
	$xml_writer->dataElement("stun", $lrow{"Stun"});
	$xml_writer->endTag();
	$xml_writer->endTag();
}

my $in_hit_data = 0;
my $in_tr = 0;
my $in_td = 0;
my $cur_col = 0;
my @text_stack = ();
my %important_tags = map { $_ => 1 } ("div", "td", "tr", "h1", "h4", "th", "thead");

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
	if (lc($tag) eq "div") {
		++$in_hit_data if ($in_hit_data > 0);
	} elsif (lc($tag) eq "thead") {
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
			$xml_writer->dataElement("hit_points", $hp);
		}
	} elsif (lc($tag) eq "td") {
		$in_td = 0;
		$data_row[$cur_col] = decode_text($text);
#		print "$cur_col: $t\n";
		++$cur_col;
	} elsif (lc($tag) eq "tr" && (@data_row > 0)) {
		process_data_row() if ($in_hit_data > 0 && $header_row[5]);
		$in_tr = 0;
	} elsif (lc($tag) eq "h4") {
		if ($text =~ /\s*(.*\S)\s+hit data/) {
			$in_hit_data = 1;
		}
	} elsif (lc($tag) eq "th") {
		push @header_row, $text;
	} elsif (lc($tag) eq "h1") {
		$xml_writer->dataElement("name", decode_text($text));
	}
}

sub text {
	my ($self, $text) = @_;
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
	$xml_writer->startTag("monster");
	my $p = HTML::Parser->new(api_version => 3,
	                          start_h => [ \&start, "self, tagname, attr, attrseq, text" ],
	                          end_h => [ \&end, "self, tagname, text" ],
	                          text_h => [ \&text, "self, text, is_cdata" ]);
	$p->parse_file($file);
	$xml_writer->endTag();
}

$xml_writer->endTag();
$xml_writer->end();
# print "$xml_writer\n";
