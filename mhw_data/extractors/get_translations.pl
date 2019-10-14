#!/usr/bin/perl

use strict;
use warnings;

use open ':std', ':encoding(UTF-8)';

use XML::Parser;
use XML::Writer;
use Data::Dumper;

my $xml_text;
my @names_stack;

my %tr_map = ();
my $index_cnt = 0;

sub handle_start
{
	my ($expat, $element, @attr) = @_;
	$xml_text = "";
	unshift @names_stack, {};
}

sub handle_end
{
	my ($expat, $element) = @_;
	if ($element =~ /name(?:_\w+)?/) {
		if (@names_stack > 1) {
			$names_stack[1]->{$element} = $xml_text;
		}
	}
	if (%{$names_stack[0]}) {
		my $nmap = { "index" => $index_cnt++ };
		my %nkeys_dedup = ();
		my %nkeys = ();
		for my $nlang (keys %{$names_stack[0]}) {
			$tr_map{$nlang} ||= {};
			$nkeys{$nlang} ||= [];
			push @{$nkeys{$nlang}}, $names_stack[0];
			my $t = $tr_map{$nlang}->{$names_stack[0]->{$nlang}};
			if ($t) {
				unless ($nkeys_dedup{$t}) {
					for my $nlang2 (keys %{$t}) {
						$nkeys{$nlang2} ||= [];
						push @{$nkeys{$nlang2}}, $t;
					}
					$nkeys_dedup{$t} = 1;
				}
				$nmap = $t if ($t->{"index"} < $nmap->{"index"});
			}
		}
		for my $nlang (keys %nkeys) {
			next if ($nlang eq "index");
			for my $i (1 .. (@{$nkeys{$nlang}} - 1)) {
				if ($nkeys{$nlang}->[0]{$nlang} ne $nkeys{$nlang}->[$i]{$nlang}) {
					print STDERR "$nlang: $nkeys{$nlang}[0]{$nlang} != $nkeys{$nlang}[$i]{$nlang}\n";
				}
			}
		}
		for my $nlang (keys %nkeys) {
			next if ($nlang eq "index");
			$nmap->{$nlang} = $nkeys{$nlang}[0]->{$nlang} unless ($nmap->{$nlang});
			$tr_map{$nlang}->{$nmap->{$nlang}} = $nmap;
		}
	}
	shift @names_stack;
}

sub handle_char
{
	my ($expat, $string) = @_;
	$xml_text .= $string;
}

for my $name (@ARGV) {
	my $p = new XML::Parser(Handlers => {
		Start => \&handle_start,
		End => \&handle_end,
		Char => \&handle_char });
	$p->parsefile($name);
}

#print Dumper(\%tr_map);

my $xml_writer;
my $io = IO::Handle->new();
$io->fdopen(fileno(STDOUT), "w");
$xml_writer = XML::Writer->new(OUTPUT => $io, ENCODING => 'utf-8', DATA_MODE => 1, DATA_INDENT => 4);
$xml_writer->xmlDecl("UTF-8");
$xml_writer->startTag("data");

#print Dumper(\%tr_map);

my %gk = ();
for my $nlang (keys %tr_map) {
	for my $v (keys %{$tr_map{$nlang}}) {
		$gk{$tr_map{$nlang}{$v}{"index"}} = $tr_map{$nlang}{$v};
	}
}
for my $k (sort { $a <=> $b } keys %gk) {
	$xml_writer->startTag("translation");
	for (sort keys %{$gk{$k}}) {
		next if ($_ eq "index");
		$xml_writer->dataElement($_, $gk{$k}->{$_});
	}
	$xml_writer->endTag();
}
$xml_writer->endTag();
