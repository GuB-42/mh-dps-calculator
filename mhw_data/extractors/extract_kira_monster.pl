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
	my @drow = @data_row;
	for (my $i = 1; $i < @drow; ++$i) { $drow[$i] =~ s/^~//; }
	$xml_writer->startTag("part");
	$xml_writer->dataElement("name", $drow[0]);
	$xml_writer->startTag("hit_data");
	$xml_writer->dataElement("cut", $drow[1]);
	$xml_writer->dataElement("impact", $drow[2]);
	$xml_writer->dataElement("bullet", $drow[3]);
	$xml_writer->dataElement("fire", $drow[4]);
	$xml_writer->dataElement("water", $drow[5]);
	$xml_writer->dataElement("thunder", $drow[6]);
	$xml_writer->dataElement("ice", $drow[7]);
	$xml_writer->dataElement("dragon", $drow[8]);
	$xml_writer->dataElement("stun", $drow[9]);
	$xml_writer->endTag();
	$xml_writer->endTag();
}

my $in_hit_data = 0;
my $in_tr = 0;
my $in_td = 0;
my $cur_col = 0;
my $text_acc = "";

sub start {
	my ($self, $tag, $attr, $attrseq, $origtext) = @_;
	if (lc($tag) eq "div") {
		++$in_hit_data if ($in_hit_data > 0);
	} elsif (lc($tag) eq "td") {
		$in_td = 1;
		$text_acc = "";
	} elsif (lc($tag) eq "tr") {
		$in_tr = 0;
		$in_td = 0;
		$cur_col = 0;
		@data_row = ();
	} elsif (lc($tag) eq "h4") {
		$text_acc = "";
	}
}

sub end {
	my ($self, $tag, $origtext) = @_;
	if (lc($tag) eq "div") {
		--$in_hit_data if ($in_hit_data > 0);
	} elsif (lc($tag) eq "td") {
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
		process_data_row() if ($in_hit_data > 0);
		$in_tr = 0;
	} elsif (lc($tag) eq "h4") {
		if ($text_acc =~ /\s*(.*\S)\s+hit data/) {
			$in_hit_data = 1;
			my $name = $1;
			$name = decode_entities($name);
			$xml_writer->dataElement("name", $name);
		}
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
