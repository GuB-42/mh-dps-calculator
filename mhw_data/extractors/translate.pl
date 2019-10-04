#!/usr/bin/perl

use strict;
use warnings;

use open ':std', ':encoding(UTF-8)';

use XML::Parser;
use XML::Twig;
use Data::Dumper;

my @xml_stack;
my @xml_string_stack;
my $cur_translation;
my @translations;
my %translation_map;

sub process_start
{
	if ($xml_stack[0] eq "translation") {
		$cur_translation = { };
	}
}
sub process_val
{
	my $val = $xml_string_stack[0];
	$val =~ s/^\s+|\s+$//g;

	if ($xml_stack[0] eq "translation") {
		push @translations, $cur_translation;
		for my $k (keys %{$cur_translation}) {
			$translation_map{$k} ||= {};
			$translation_map{$k}{$cur_translation->{$k}} ||= [];
			push @{$translation_map{$k}{$cur_translation->{$k}}}, $cur_translation;
		}
	} elsif ($xml_stack[1] eq "translation") {
		$cur_translation->{$xml_stack[0]} = $val;
	}
}
sub handle_start
{
	my ($expat, $element, @attr) = @_;
	unshift @xml_stack, $element;
	unshift @xml_string_stack, "";
	process_start();
}
sub handle_end
{
	my ($expat, $element) = @_;
	process_val();
	shift @xml_stack;
	shift @xml_string_stack;
}
sub handle_char
{
	my ($expat, $string) = @_;
	$xml_string_stack[0] .= $string;
}

my $trans_file = shift @ARGV;

my $p = new XML::Parser(Handlers => {
	Start => \&handle_start,
	End => \&handle_end,
	Char => \&handle_char });
@xml_string_stack = ();
@xml_stack = ("null", "null", "null");
$p->parsefile($trans_file);

sub translate_func
{
	my $elt = $_[0];
	my $indent = "";
	my @trans_list = ();
	my @trans_list_nsuff = ();
	my %trans_set = ();
	my %trans_vals = ();
	my %trans_vals_nsuff = ();
	my $first_elt;
	my $suffix = "";

	for my $elpc ($elt->parent()->children()) {
		my $tag = $elpc->tag();
		if ($translation_map{$tag}) {
			$first_elt = $elpc unless (defined($first_elt));
			my $text = $elpc->text();
			if ($elpc->prev_sibling()->tag() eq "#PCDATA") {
				$indent = $elpc->prev_sibling()->text();
			}
			$trans_set{$tag} = $elpc;
			$trans_vals{$tag} = $text;
			if ($translation_map{$tag}->{$text}) {
				push @trans_list, @{$translation_map{$tag}->{$text}}
			}

			my $btext = $text;
			if ($btext =~ /(\s+\[\d+\])$/) {
				my $suff = $1;
				$btext =~ s/(\s+\[\d+\])$//;
				$trans_vals_nsuff{$tag} = $btext;
				if ($translation_map{$tag}->{$btext}) {
					push @trans_list_nsuff, @{$translation_map{$tag}->{$btext}};
					$suffix = $suff;
				}
			} else {
				$trans_vals_nsuff{$tag} = $text;
			}

		}
	}

	if (@trans_list) {
		$suffix = "";
	} else {
		@trans_list = @trans_list_nsuff;
		%trans_vals = %trans_vals_nsuff;
	}

	my $sel_trans;
	for my $trans (@trans_list) {
		my $ok = 1;
		for my $k (keys %trans_vals) {
			if (defined $trans->{$k} && $trans->{$k} ne $trans_vals{$k}) {
				$ok = 0;
				last;
			}
		}
		if ($ok) {
			$sel_trans = $trans;
			last;
		}
	}
	if (defined $sel_trans) {
		for my $k (keys %{$sel_trans}) {
			$trans_set{$k} = 0 unless (defined($trans_set{$k}));
		}
		my $last_elt;
		for my $k (sort keys %trans_set) {
			if ($trans_set{$k}) {
				$last_elt = $trans_set{$k};
			} else {
				my $ielt = XML::Twig::Elt->new("#PCDATA" => $indent);
				my $elt = XML::Twig::Elt->new($k => ($sel_trans->{$k} . $suffix));
				if ($last_elt) {
					$elt->paste(after => $last_elt);
					$ielt->paste(before => $elt);
				} else {
					$elt->paste(before => $first_elt);
					$ielt->paste(after => $elt);
				}
				$last_elt = $elt;
			}
		}
	}
#	print Dumper($sel_trans);
#	print "indent: \"$indent\"\n";
#	print Dumper(\@trans_list);
#	print Dumper(\%trans_vals);
#	print "------------------------------------------\n";
}

my $twig = XML::Twig->new(keep_spaces => 1,
                          empty_tags => 'normal',
                          comments => 'keep');
for my $f (@ARGV) {
	$twig->parsefile($f);
	for my $name_key (keys %translation_map) {
		for my $elt ($twig->root->descendants($name_key)) {
			translate_func($elt);
		}
	}
	$twig->print();
}

