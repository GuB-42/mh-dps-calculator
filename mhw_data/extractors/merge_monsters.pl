#!/usr/bin/perl

use strict;
use warnings;

use open ':std', ':encoding(UTF-8)';
use utf8;

use XML::Parser;
use XML::Writer;
use Data::Dumper;

my @xml_stack;
my @xml_string_stack;
my @xml_stack_attr;

my $cur_translation;
my $cur_monster;
my $cur_part;
my $cur_hit_data;
my $cur_tolerance;

my @monsters;
my @translations;
my %translation_map;

sub process_start
{
	if ($xml_stack[0] eq "monster") {
		$cur_monster = {
			"tolerances" => {},
			"parts" => []
		};
	} elsif ($xml_stack[0] eq "part") {
		$cur_part = {
			"hit_data" => {}
		};
	} elsif ($xml_stack[0] eq "hit_data") {
		$cur_hit_data = { };
	} elsif ($xml_stack[0] eq "tolerance") {
		$cur_tolerance = { };
	} elsif ($xml_stack[0] eq "translation") {
		$cur_translation = { };
	}
}

sub process_val
{
	my $val = $xml_string_stack[0];
	$val =~ s/^\s+|\s+$//g;

	if ($xml_stack[0] eq "monster") {
		$cur_monster->{"index"} = scalar @monsters;
		push @monsters, $cur_monster;
	} if ($xml_stack[1] eq "monster") {
		if ($xml_stack[0] eq "part") {
			push @{$cur_monster->{"parts"}}, $cur_part;
		} elsif ($xml_stack[0] eq "tolerance") {
			$cur_monster->{"tolerances"}{$cur_tolerance->{"type"}} = $cur_tolerance;
		} else {
			$cur_monster->{$xml_stack[0]} = $val;
		}
	} elsif ($xml_stack[2] eq "monster" && $xml_stack[1] eq "part") {
		if ($xml_stack[0] eq "hit_data") {
			$cur_part->{"hit_data"}{$cur_hit_data->{"state"} || ""} = $cur_hit_data;
		} else {
			$cur_part->{$xml_stack[0]} = $val;
		}
	} elsif ($xml_stack[2] eq "part" && $xml_stack[1] eq "hit_data") {
		$cur_hit_data->{$xml_stack[0]} = $val;
	} elsif ($xml_stack[2] eq "monster" && $xml_stack[1] eq "tolerance") {
		$cur_tolerance->{$xml_stack[0]} = $val;
	} elsif ($xml_stack[0] eq "translation") {
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
	my $attr_map = {};
	for (my $i = 0; $i < @attr; $i += 2) {
		$attr_map->{$attr[$i]} = $attr[$i + 1];
	}
	unshift @xml_stack_attr, $attr_map;
	process_start();
}
sub handle_end
{
	my ($expat, $element) = @_;
	process_val();
	shift @xml_stack;
	shift @xml_string_stack;
	shift @xml_stack_attr;
}
sub handle_char
{
	my ($expat, $string) = @_;
	$xml_string_stack[0] .= $string;
}

my %hardcoded_match = (
	"Back Bone" => { "胴と脚の骨" => 1 },
	"Left Hindleg Bone" => { "胴と脚の骨" => 1 },
	"Right Hindleg Bone" => { "胴と脚の骨" => 1 }
);

sub find_other_part
{
	my ($part_a, $parts_b) = @_;

	my %part_a_names;
	my @part_a_trans;
	my @part_a_trans_left_right;
	my @part_a_trans_sub;
	for my $k (keys %{$part_a}) {
		if ($k =~ /^name/) {
			$part_a_names{$k} = $part_a->{$k};
		}

		if ($translation_map{$k}) {
			my $n = $part_a->{$k};
			if ($translation_map{$k}->{$n}) {
				push @part_a_trans, @{$translation_map{$k}->{$n}};
				push @part_a_trans_left_right, @{$translation_map{$k}->{$n}};
			}
			if ($n =~ /^(?:Left|Right) (.*)/) {
				if ($translation_map{$k}->{$1}) {
					push @part_a_trans_left_right, @{$translation_map{$k}->{$1}};
				}
			}
			if ($n =~ /(.*\S)\s*\(.*\)$/) {
				if ($translation_map{$k}->{$1}) {
					push @part_a_trans_sub, @{$translation_map{$k}->{$1}};
				}
			}
		}
	}
	my $part_a_pnames = (join ' / ', values %part_a_names);

	my $other_part;
	for my $part_b (@{$parts_b}) {
		for my $k (keys %{$part_b}) {
			if (defined $part_a_names{$k} && $part_a_names{$k} eq $part_b->{$k}) {
				$other_part = $part_b;
				last;
			}
		}
		last if ($other_part);
	}
	unless ($other_part) {
		for my $part_b (@{$parts_b}) {
			for my $k (keys %{$part_b}) {
				for my $trans (@part_a_trans) {
					if (defined $trans->{$k} && $trans->{$k} eq $part_b->{$k}) {
						$other_part = $part_b;
						last;
					}
				}
				last if ($other_part);
			}
			last if ($other_part);
		}
	}
	unless ($other_part) {
		for my $part_b (@{$parts_b}) {
			for my $k (keys %{$part_b}) {
				if (defined $part_a_names{$k} &&
				    ("Left $part_a_names{$k}" eq $part_b->{$k} ||
				     "Right $part_a_names{$k}" eq $part_b->{$k} ||
				     $part_a_names{$k} eq "Left $part_b->{$k}" ||
				     $part_a_names{$k} eq "Right $part_b->{$k}")) {
					$other_part = $part_b;
					last;
				}
			}
			last if ($other_part);
		}
	}
	unless ($other_part) {
		for my $part_b (@{$parts_b}) {
			for my $k (keys %{$part_b}) {
				for my $trans (@part_a_trans_left_right) {
					if (defined $trans->{$k} &&
					    ($trans->{$k} eq $part_b->{$k} ||
					     "Left $trans->{$k}" eq $part_b->{$k} ||
					     "Right $trans->{$k}" eq $part_b->{$k} ||
					     $trans->{$k} eq "Left $part_b->{$k}" ||
					     $trans->{$k} eq "Right $part_b->{$k}")) {
						$other_part = $part_b;
						last;
					}
				}
				last if ($other_part);
			}
			last if ($other_part);
		}
	}
	unless ($other_part) {
		for my $part_b (@{$parts_b}) {
			for my $k (keys %{$part_b}) {
				if (defined $part_a_names{$k}) {
					my $subname_a = $part_a_names{$k};
					$subname_a =~ s/\s*\(.*\)$//;
					my $subname_b = $part_b->{$k};
					$subname_b =~ s/\s*\(.*\)$//;
					if ($subname_a eq $part_b->{$k} ||
					    $part_a_names{$k} eq $subname_b) {
						$other_part = $part_b;
						last;
					}
				}
			}
			last if ($other_part);
		}
	}
	unless ($other_part) {
		for my $part_b (@{$parts_b}) {
			for my $k (keys %{$part_b}) {
				for my $trans (@part_a_trans_sub) {
					if (defined $trans->{$k}) {
						my $subname_a = $trans->{$k};
						$subname_a =~ s/\s*\(.*\)$//;
						my $subname_b = $part_b->{$k};
						$subname_b =~ s/\s*\(.*\)$//;
						if ($subname_a eq $part_b->{$k} ||
						    $trans->{$k} eq $subname_b) {
							$other_part = $part_b;
							last;
						}
					}
				}
				last if ($other_part);
			}
			last if ($other_part);
		}
	}
	unless ($other_part) {
		for my $k2 (keys %part_a_names) {
			my $a_name = $part_a_names{$k2};
			for my $part_b (@{$parts_b}) {
				for my $k (keys %{$part_b}) {
					next unless ($k =~ /^name/);
					if (defined $hardcoded_match{$part_b->{$k}} &&
					    $hardcoded_match{$part_b->{$k}}->{$a_name}) {
						$other_part = $part_b;
						last;
					}
				}
				last if ($other_part);
			}
			last if ($other_part);
		}
	}
	return $other_part;
}

sub merge_parts
{
	my ($parts_a, $parts_b, $monster_names) = @_;
	my $ret = [];

	my @used_parts_b;

	for my $part_a (@{$parts_a}) {
		my @part_a_pnames_tab = ();
		for my $k (keys %{$part_a}) {
			if ($k =~ /^name/) {
				push @part_a_pnames_tab, $part_a->{$k};
			}
		}
		my $part_a_pnames = (join ' / ', @part_a_pnames_tab);


		my $other_part = find_other_part($part_a, $parts_b);

		if ($other_part) {
			my %other_part_names;
			for my $k (keys %{$other_part}) {
				if ($k =~ /^name/) {
					$other_part_names{$k} = $other_part->{$k};
				}
			}
			my $other_part_pnames = (join ' / ', values %other_part_names);

			my $ret_part = {
				"hit_data" => { }
			};

			for my $k (keys %{$part_a}) {
				if (defined $other_part->{$k}) {
					if ($k eq "hit_data") {
						for my $state (keys %{$part_a->{"hit_data"}}) {
							if (defined $other_part->{"hit_data"}{$state}) {
								$ret_part->{"hit_data"}{$state} = {};
								for my $kd (keys %{$part_a->{"hit_data"}{$state}}) {
									if (defined $other_part->{"hit_data"}{$state}{$kd}) {
										if ($part_a->{"hit_data"}{$state}{$kd} ne $other_part->{"hit_data"}{$state}{$kd}) {
											print STDERR "$monster_names: part hit data mismatch: " .
												"$part_a_pnames [$state.$kd] ($part_a->{hit_data}{$state}{$kd}) != " .
												"$other_part_pnames [$state.$kd] ($other_part->{hit_data}{$state}{$kd})\n";
										}
										$ret_part->{"hit_data"}{$state}{$kd} = $other_part->{"hit_data"}{$state}{$kd};
									} else {
										$ret_part->{"hit_data"}{$state}{$kd} = $part_a->{"hit_data"}{$state}{$kd};
									}
								}
								for my $kd (keys %{$other_part->{"hit_data"}{$state}}) {
									unless (defined $part_a->{"hit_data"}{$state}{$kd}) {
										$ret_part->{"hit_data"}{$state}{$kd} = $other_part->{"hit_data"}{$state}{$kd};
									}
								}
							} else {
								print STDERR "$monster_names: part state mismatch: " .
									"$part_a_pnames [$state] != $other_part_pnames [<null>]\n";
								$ret_part->{"hit_data"}{$state} = $part_a->{"hit_data"}{$state};
							}
						}
						for my $state (keys %{$other_part->{"hit_data"}}) {
							unless (defined $part_a->{"hit_data"}{$state}) {
								print STDERR "$monster_names: part state mismatch: " .
									"$part_a_pnames [$state] != $other_part_pnames [$state]\n";
								$ret_part->{"hit_data"}{$state} = $other_part->{"hit_data"}{$state};
							}
						}
					} else {
						if ($part_a->{$k} ne $other_part->{$k}) {
							print STDERR "$monster_names: part data mismatch: " .
								"$part_a_pnames [$k] != $other_part_pnames [$k]\n";
						}
						$ret_part->{$k} = $other_part->{$k};
					}
				} else {
					$ret_part->{$k} = $part_a->{$k};
				}
			}
			for my $k (keys %{$other_part}) {
				unless (defined $part_a->{$k}) {
					$ret_part->{$k} = $other_part->{$k};
				}
			}
			push @used_parts_b, $other_part;
			push @{$ret}, $ret_part;
		} else {
			print STDERR "$monster_names: part mismatch: $part_a_pnames != <null>\n";
			push @{$ret}, $part_a;
		}
	}
	for my $part_b (@{$parts_b}) {
		my %part_b_names;
		for my $k (keys %{$part_b}) {
			if ($k =~ /^name/) {
				$part_b_names{$k} = $part_b->{$k};
			}
		}
		my $part_b_pnames = (join ' / ', values %part_b_names);

		my $found = 0;
		for my $used_part_b (@used_parts_b) {
			if ($used_part_b == $part_b) {
				$found = 1;
				last;
			}
		}
		unless ($found) {
			print STDERR "$monster_names: part mismatch: <null> != $part_b_pnames\n";
			push @{$ret}, $part_b;
		}
	}

	return $ret;
}

sub merge_monsters
{
	my ($monster_a, $monster_b) = @_;

	my $ret = {
		"tolerances" => {},
		"parts" => []
	};

	my @name_list = ();
	for my $k (keys %{$monster_a}) {
		push @name_list, $monster_a->{$k} if ($k =~ /^name/);
	}
	for my $k (keys %{$monster_b}) {
		push @name_list, $monster_b->{$k} if ($k =~ /^name/);
	}
	my $names = (join " / ", @name_list);

	for my $k (keys %{$monster_a}) {
		if (defined $monster_b->{$k}) {
			if ($k eq "tolerances") {
				for my $tol_k (keys %{$monster_a->{$k}}) {
					if (defined $monster_b->{$k}{$tol_k}) {
						$ret->{$k}{$tol_k} = {};
						for my $tol_data ("initial", "plus", "max", "regen_value", "regen_tick", "duration", "damage") {
							if (defined $monster_a->{$k}{$tol_k}{$tol_data}) {
								if (defined $monster_b->{$k}{$tol_k}{$tol_data}) {
									if ($monster_a->{$k}{$tol_k}{$tol_data} ne $monster_b->{$k}{$tol_k}{$tol_data}) {
										print STDERR "$names: $k.$tol_k.$tol_data mismatch: " .
											"$monster_a->{$k}{$tol_k}{$tol_data} != $monster_b->{$k}{$tol_k}{$tol_data}\n";
									}
									$ret->{$k}{$tol_k}{$tol_data} = $monster_a->{$k}{$tol_k}{$tol_data};
								} else {
									print STDERR "$names: $k.$tol_k.$tol_data mismatch: $monster_a->{$k}{$tol_k}{$tol_data} != <null>\n";
									$ret->{$k}{$tol_k}{$tol_data} = $monster_a->{$k}{$tol_k}{$tol_data};
								}
							} elsif (defined $monster_b->{$k}{$tol_k}{$tol_data}) {
								print STDERR "$names: $k.$tol_k.$tol_data mismatch: <null> != $monster_b->{$k}{$tol_k}{$tol_data}\n";
								$ret->{$k}{$tol_k}{$tol_data} = $monster_b->{$k}{$tol_k}{$tol_data};
							}
						}
					} else {
						$ret->{$k}{$tol_k} = $monster_a->{$k}{$tol_k};
					}
				}
				for my $tol_k (keys %{$monster_b->{$k}}) {
					unless (defined $monster_a->{$k}{$tol_k}) {
						$ret->{$k}{$tol_k} = $monster_b->{$k}{$tol_k};
					}
				}
			} elsif ($k eq "parts") {
				$ret->{$k} = merge_parts($monster_a->{$k}, $monster_b->{$k}, $names);
			} elsif ($k eq "index") {
				$ret->{$k} = $monster_a->{$k} < $monster_b->{$k} ? $monster_a->{$k} : $monster_b->{$k};
			} else {
				if ($monster_a->{$k} ne $monster_b->{$k}) {
					print STDERR "$names: $k mismatch: $monster_a->{$k} != $monster_b->{$k}\n";
				}
				$ret->{$k} = $monster_a->{$k};
			}
		} else {
			$ret->{$k} = $monster_a->{$k};
		}
	}
	for my $k (keys %{$monster_b}) {
		unless (defined $monster_a->{$k}) {
			$ret->{$k} = $monster_b->{$k};
		}
	}

	return $ret;
}

for my $k (keys %hardcoded_match) {
	for my $o (keys %{$hardcoded_match{$k}}) {
		$hardcoded_match{$o} ||= {};
		$hardcoded_match{$o}->{$k} = 1;
	}
}

for my $file (@ARGV) {
	my $p = new XML::Parser(Handlers => {
		Start => \&handle_start,
		End => \&handle_end,
		Char => \&handle_char });
	@xml_string_stack = ();
	@xml_stack = ("null", "null", "null");
	$p->parsefile($file);
}

my %groups = ();
my @unnamed;
for my $monster (@monsters) {
	if (defined $monster->{"name"}) {
		$groups{$monster->{"name"}} ||= [];
		push @{$groups{$monster->{"name"}}}, $monster;
	} else {
		my $found = 0;
		for my $k (keys %translation_map) {
			if (defined $monster->{$k} &&
			    defined $translation_map{$k}->{$monster->{$k}}[0]{"name"}) {
				push @{$groups{$translation_map{$k}->{$monster->{$k}}[0]{"name"}}}, $monster;
				$found = 1;
				last;
			}
		}
		unless ($found) {
			for my $k (keys %{$monster}) {
				if ($k =~ /^name/) {
					push @{$groups{$monster->{$k}}}, $monster;
					$found = 1;
					last;
				}
			}
		}
		unless ($found) {
			push @unnamed, $monster;
		}
	}
}

for my $k (sort keys %groups) {
	my $acc = { "index" => -1 };
	my $first = 1;
	for my $monster (@{$groups{$k}}) {
		if ($first) {
			$acc = $monster;
			$first = 0;
		} else {
			$acc = merge_monsters($acc, $monster);
		}
	}
	if ($translation_map{"name"}{$acc->{"name"}}) {
		for my $tk (keys %{$translation_map{"name"}{$acc->{"name"}}[0]}) {
			$acc->{$tk} = $translation_map{"name"}{$acc->{"name"}}[0]{$tk};
		}
	}
	$groups{$k} = $acc;
}

my $xml_writer;

sub print_monster
{
	my ($monster) = @_;

	my $monster_id = lc($monster->{"name"});
	$monster_id =~ s/\W+/_/g;
	$xml_writer->startTag("monster", "id" => $monster_id);
	$xml_writer->dataElement("name", $monster->{"name"});
	$xml_writer->dataElement("name_fr", $monster->{"name_fr"}) if ($monster->{"name_fr"});
	$xml_writer->dataElement("name_jp", $monster->{"name_jp"}) if ($monster->{"name_jp"});
	$xml_writer->dataElement("hit_points", $monster->{"hit_points"}) if ($monster->{"hit_points"});
	for my $part (@{$monster->{"parts"}}) {
		if ($part->{"name_jp"} && !$part->{"name"}) {
			if ($translation_map{"name_jp"}{$part->{"name_jp"}}) {
				for my $trans (@{$translation_map{"name_jp"}{$part->{"name_jp"}}}) {
					if ($trans->{"name"}) {
						$part->{"name"} = $trans->{"name"};
						last;
					}
				}
			}
		}
		my $part_id = lc($part->{"name"});
		$part_id =~ s/\W+/_/g;
		$part_id =~ s/^_|_$//g;
		$xml_writer->startTag("part", "id" => $part_id);
		$xml_writer->dataElement("name", $part->{"name"}) if ($part->{"name"});
		$xml_writer->dataElement("name_fr", $part->{"name_fr"}) if ($part->{"name_fr"});
		$xml_writer->dataElement("name_jp", $part->{"name_jp"}) if ($part->{"name_jp"});
		for my $state (sort keys %{$part->{"hit_data"}}) {
			$xml_writer->startTag("hit_data");
			$xml_writer->dataElement("state", $state) if ($state);
			for my $key ("cut", "impact", "bullet", "fire", "water", "thunder", "ice", "dragon", "stun") {
				$xml_writer->dataElement($key, $part->{"hit_data"}{$state}{$key});
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

my $io = IO::Handle->new();
$io->fdopen(fileno(STDOUT), "w");
$xml_writer = XML::Writer->new(OUTPUT => $io, ENCODING => 'utf-8', DATA_MODE => 1, DATA_INDENT => 4);
$xml_writer->xmlDecl("UTF-8");
$xml_writer->startTag("data");
for my $wkey (sort { $groups{$a}->{"index"} <=> $groups{$b}->{"index"} } (keys %groups)) {
	print_monster($groups{$wkey});
}
$xml_writer->endTag();
