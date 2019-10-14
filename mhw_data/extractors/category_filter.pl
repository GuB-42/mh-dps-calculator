#!/usr/bin/perl

use strict;
use warnings;

use open ':std', ':encoding(UTF-8)';

use XML::Twig;
use Data::Dumper;

my $category = shift @ARGV;

my $twig = XML::Twig->new(keep_spaces => 1,
                          empty_tags => 'normal',
                          comments => 'keep');
for my $f (@ARGV) {
	$twig->parsefile($f);
	for my $elt ($twig->root->descendants("categories")) {
		my %cats = ();
		for my $elc ($elt->children("category_ref")) {
			$cats{$elc->id} = 1;
		}
		unless ($cats{$category}) {
			$elt->parent->delete();
		}
	}
	$twig->print();
}
