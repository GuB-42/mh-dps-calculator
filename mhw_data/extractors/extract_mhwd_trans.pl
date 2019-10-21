#!/usr/bin/perl

use strict;
use warnings;

use Text::CSV;
use XML::Writer;
use Data::Dumper;

my $io = IO::Handle->new();
$io->fdopen(fileno(STDOUT), "w");
my $xml_writer = XML::Writer->new(OUTPUT => $io, ENCODING => 'utf-8', DATA_MODE => 1, DATA_INDENT => 4);
$xml_writer->xmlDecl("UTF-8");

my %col_rename = (
	"name_en" => "name",
	"name_ja" => "name_jp",
);

$xml_writer->startTag("data");
for my $file (@ARGV) {
	my $csv = Text::CSV->new( { binary => 1 } ) ||
		die "Cannot use CSV: ".Text::CSV->error_diag();
	open my $fh, "<:encoding(utf8)", $file || die "$file: $!";
	my $first = 1;
	my @header = ();
	my @order = ();
	while (my $row = $csv->getline($fh)) {
		if ($first) {
			@header = @{$row};
			@header = map { $col_rename{$_} || $_ } @header;
			@order = sort @header;
			$first = 0;
		} else {
			my $hline = {};
			for (my $i = 0; $i < scalar @{$row} && $i < scalar @header; ++$i) {
				$hline->{$header[$i]} = $row->[$i];
			}
			$xml_writer->startTag("translation");
			for (@order) {
				next unless ($hline->{$_});
				next unless ($_ =~ /^name/);
#				next if ($_ ne "name" && defined $hline->{$_} && $hline->{$_} eq $hline->{"name"});
				$xml_writer->dataElement($_, $hline->{$_});
			}
			$xml_writer->endTag();
		}
	}

	$csv->eof || $csv->error_diag();
	close $fh;
}
$xml_writer->endTag();
$xml_writer->end();

