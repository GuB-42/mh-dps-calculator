#!/usr/bin/perl

use strict;
use warnings;

my %h = ();

open F1, "<", $ARGV[0] || die;
while (<F1>) {
	s/^\s*|\s*$//g;
	$h{$_} = 1;
}
close F1;

open F2, "<", $ARGV[1] || die;
while (<F2>) {
	my $l = $_;
	s/^\s*|\s*$//g;
	print $l unless ($h{$_});
}
close F2;
