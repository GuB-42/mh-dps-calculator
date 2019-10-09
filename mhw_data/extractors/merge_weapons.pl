#!/usr/bin/perl

use strict;
use warnings;

use open ':std', ':encoding(UTF-8)';

use XML::Parser;
use XML::Writer;
use Data::Dumper;

my @xml_stack;
my @xml_string_stack;
my @xml_stack_attr;

my $cur_translation;
my $cur_weapon;
my $cur_ammo = {};
my $cur_file_name;

my @weapons;
my @translations;
my %translation_map;

sub process_start
{
	if ($xml_stack[0] eq "weapon") {
		$cur_weapon = {
			"file_name" => $cur_file_name,
			"elements" => {},
			"sharpness" => {},
			"phial_elements" => {},
			"slots" => [],
			"ammos" => [],
			"notes" => [],
			"categories" => {}
		};
	} elsif ($xml_stack[0] eq "translation") {
		$cur_translation = {};
	}
}

sub process_val
{
	my $val = $xml_string_stack[0];
	$val =~ s/^\s+|\s+$//g;

	if ($xml_stack[0] eq "weapon") {
		$cur_weapon->{"index"} = scalar @weapons;
		push @weapons, $cur_weapon;
	} if ($xml_stack[1] eq "weapon") {
		unless ($xml_stack[0] eq "element" ||
		        $xml_stack[0] eq "sharpness" ||
		        $xml_stack[0] eq "phial" ||
		        $xml_stack[0] eq "slots" ||
		        $xml_stack[0] eq "ammos" ||
		        $xml_stack[0] eq "notes" ||
		        $xml_stack[0] eq "categories") {
			if ($xml_stack[0] eq "weapon_type_ref") {
				$cur_weapon->{$xml_stack[0]} = $xml_stack_attr[0]{"id"};
			} else {
		        $cur_weapon->{$xml_stack[0]} = $val;
		    }
		}
	} elsif ($xml_stack[2] eq "weapon" && $xml_stack[1] eq "element") {
		$cur_weapon->{"elements"}{$xml_stack[0]} = $val;
	} elsif ($xml_stack[2] eq "weapon" && $xml_stack[1] eq "sharpness") {
		$cur_weapon->{"sharpness"}{$xml_stack[0]} = $val;
	} elsif ($xml_stack[2] eq "weapon" && $xml_stack[1] eq "phial") {
		if ($xml_stack[0] eq "impact" ||
		    $xml_stack[0] eq "element" ||
		    $xml_stack[0] eq "power") {
			$cur_weapon->{"phial"} = $xml_stack[0];
		} else {
			$cur_weapon->{"phial_elements"}{$xml_stack[0]} = $val;
		}
	} elsif ($xml_stack[2] eq "weapon" && $xml_stack[1] eq "slots" && $xml_stack[0] eq "slot") {
		push @{$cur_weapon->{"slots"}}, $val;
	} elsif ($xml_stack[2] eq "weapon" && $xml_stack[1] eq "ammos" && $xml_stack[0] eq "ammo_ref") {
		$cur_ammo->{"id"} = $xml_stack_attr[0]{"id"};
		push @{$cur_weapon->{"ammos"}}, $cur_ammo;
		$cur_ammo = {};
	} elsif ($xml_stack[3] eq "weapon" && $xml_stack[2] eq "ammos" && $xml_stack[1] eq "ammo_ref") {
		$cur_ammo->{$xml_stack[0]} = $val;
	} elsif ($xml_stack[2] eq "weapon" && $xml_stack[1] eq "notes") {
		push @{$cur_weapon->{"notes"}}, $xml_stack[0];
	} elsif ($xml_stack[2] eq "weapon" && $xml_stack[1] eq "categories" && $xml_stack[0] eq "category_ref") {
		$cur_weapon->{"categories"}{$xml_stack_attr[0]{"id"}} = 1;
	} elsif ($xml_stack[0] eq "translation") {
		push @translations, $cur_translation;
		for my $k (keys %{$cur_translation}) {
			$translation_map{$k} ||= {};
			$translation_map{$k}{$cur_translation->{$k}} = $cur_translation;
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

sub merge_weapons
{
	my ($wep_a, $wep_b) = @_;
	my $ret = {
		"elements" => {},
		"sharpness" => {},
		"phial_elements" => {},
		"slots" => []
	};

	my @name_list = ();
	my $name_a;
	if (defined $wep_a->{"name"}) {
		$name_a = $wep_a->{"name"};
	} else {
		$name_a = join " / ", (map { $_ =~ /^name/ ? ( $wep_a->{$_} ) : () } (keys %{$wep_a}))
	}
	my $name_b;
	if (defined $wep_b->{"name"}) {
		$name_b = $wep_b->{"name"};
	} else {
		$name_b = join " / ", (map { $_ =~ /^name/ ? ( $wep_b->{$_} ) : () } (keys %{$wep_b}))
	}
	my $names = "$wep_a->{weapon_type_ref}: $wep_a->{file_name}\[$name_a\] / $wep_b->{file_name}\[$name_b\]";

	for my $k ("affinity", "awakened", "slots") {
		if (defined $wep_a->{$k} && !defined $wep_b->{$k}) {
			print STDERR "$names: $k mismatch: $wep_a->{$k} != <null>\n";
		}
		if (!defined $wep_a->{$k} && defined $wep_b->{$k}) {
			print STDERR "$names: $k mismatch: <null> != $wep_b->{$k}\n";
		}
	}

	for my $k (keys %{$wep_a}) {
		if (defined $wep_b->{$k}) {
			if ($k eq "elements" || $k eq "phial_elements") {
				for my $e (keys %{$wep_a->{$k}}) {
					if (defined $wep_b->{$k}{$e}) {
						if ($wep_a->{$k}{$e} ne $wep_b->{$k}{$e}) {
							print STDERR "$names: $k/$e mismatch: $wep_a->{$k}{$e} != $wep_b->{$k}{$e}\n";
						}
					} else {
						print STDERR "$names: $k/$e mismatch: $wep_a->{$k}{$e} != <null>\n";
					}
					$ret->{$k}{$e} = $wep_a->{$k}{$e};
				}
				for my $e (keys %{$wep_b->{$k}}) {
					unless (defined $wep_a->{$k}{$e}) {
						print STDERR "$names: $k/$e mismatch: <null> != $wep_b->{$k}{$e}\n";
					}
				}
			} elsif ($k eq "slots") {
				my @sa = (sort { $b cmp $a } @{$wep_a->{$k}});
				my @sb = (sort { $b cmp $a } @{$wep_b->{$k}});
				my $mismatch = 0;
				if (@sa == @sb) {
					for (my $i = 0; $i < @sa; ++$i) {
						$mismatch = 1 if ($sa[$i] ne $sb[$i]);
						last;
					}
				} else {
					$mismatch = 1;
				}
				if ($mismatch) {
					print STDERR "$names: $k mismatch: [" . (join ',', @sa) . "] != [" . (join ',', @sb) . "]\n";
				}
				$ret->{$k} = $wep_a->{$k};
			} elsif ($k eq "notes") {
				if (!@{$wep_b->{$k}}) {
					$ret->{$k} = $wep_a->{$k};
				} elsif (!@{$wep_a->{$k}}) {
					$ret->{$k} = $wep_b->{$k};
				} else {
					my @sa = (sort { $b cmp $a } @{$wep_a->{$k}});
					my @sb = (sort { $b cmp $a } @{$wep_b->{$k}});
					my $mismatch = 0;
					if (@sa == @sb) {
						for (my $i = 0; $i < @sa; ++$i) {
							$mismatch = 1 if ($sa[$i] ne $sb[$i]);
							last;
						}
					} else {
						$mismatch = 1;
					}
					if ($mismatch) {
						print STDERR "$names: $k mismatch: [" . (join ',', @sa) . "] != [" . (join ',', @sb) . "]\n";
					}
					$ret->{$k} = $wep_a->{$k};
				}
			} elsif ($k eq "ammos") {
				if (!@{$wep_b->{$k}}) {
					$ret->{$k} = $wep_a->{$k};
				} elsif (!@{$wep_a->{$k}}) {
					$ret->{$k} = $wep_b->{$k};
				} else {
					my @sa = (sort { $b->{"id"} cmp $a->{"id"} } @{$wep_a->{$k}});
					my @sb = (sort { $b->{"id"} cmp $a->{"id"} } @{$wep_b->{$k}});
					my $mismatch = 0;
					if (@sa == @sb) {
						for (my $i = 0; $i < @sa; ++$i) {
							$mismatch = 1 if ($sa[$i]->{"id"} ne $sb[$i]->{"id"});
							last;
						}
					} else {
						$mismatch = 1;
					}
					if ($mismatch) {
						print STDERR "$names: $k mismatch: [" .
							(join ',', (map { $_->{"id"} } @sa)) .
							"] != [" .
							(join ',', (map { $_->{"id"} } @sb)) .
							"]\n";
					}

					my %b_id_map = ();
					for my $ammo (@{$wep_b->{$k}}) {
						$b_id_map{$ammo->{"id"}} = $ammo;
					}
					$ret->{$k} = [];
					for my $ammo (@{$wep_a->{$k}}) {
						my $other_ammo = $b_id_map{$ammo->{"id"}};
						if ($other_ammo) {
							my $ret_ammo = {};
							for my $ak (keys %{$ammo}) {
								if ($ammo->{$ak} ne $other_ammo->{$ak}) {
									print STDERR "$names: $k/$ammo->{id}/$ak mismatch: $ammo->{$ak} != $other_ammo->{$ak}\n";
								}
								$ret_ammo->{$ak} = $ammo->{$ak};
							}
							for my $ak (keys %{$other_ammo}) {
								$ret_ammo->{$ak} = $other_ammo->{$ak} unless (defined $ammo->{$ak});
							}
							push @{$ret->{$k}}, $ret_ammo;
						} else {
							push @{$ret->{$k}}, $ammo;
						}
					}
				}
			} elsif ($k eq "categories") {
				$ret->{$k} = $wep_a->{$k};
				for my $cat (keys %{$wep_b->{$k}}) {
					$ret->{$k}{$cat} = 1;
				}
			} elsif ($k eq "sharpness") {
				if (!%{$wep_b->{$k}}) {
					$ret->{$k} = $wep_a->{$k};
				} elsif (!%{$wep_a->{$k}}) {
					$ret->{$k} = $wep_b->{$k};
				} else {
					my %sharpness_levels = (
						"red" => 0,
						"orange" => 1,
						"yellow" => 2,
						"green" => 3,
						"blue" => 4,
						"white" => 5,
						"purple" => 6
					);
					my @sha = ( 0, 0, 0, 0, 0, 0, 0 );
					my $sha_plus = 0;
					my @shb = ( 0, 0, 0, 0, 0, 0, 0 );
					my $shb_plus = 0;
					for my $sh (keys %{$wep_a->{$k}}) {
						if (defined $sharpness_levels{$sh}) {
							$sha[$sharpness_levels{$sh}] = $wep_a->{$k}{$sh};
						} elsif ($sh eq "plus") {
							$sha_plus = $wep_a->{$k}{$sh};
						} elsif (!defined $wep_b->{$k}{$sh}) {
							print STDERR "$names: $k/$sh mismatch: $wep_a->{$k}{$sh} != <null>\n";
						} elsif ($wep_a->{$k}{$sh} ne $wep_b->{$k}{$sh}) {
							print STDERR "$names: $k/$sh mismatch: $wep_a->{$k}{$sh} != $wep_b->{$k}{$sh}\n";
						}
					}
					for my $sh (keys %{$wep_b->{$k}}) {
						if (defined $sharpness_levels{$sh}) {
							$shb[$sharpness_levels{$sh}] = $wep_b->{$k}{$sh};
						} elsif ($sh eq "plus") {
							$shb_plus = $wep_b->{$k}{$sh};
						} elsif (!defined $wep_a->{$k}{$sh}) {
							print STDERR "$names: $k/$sh mismatch: <null> != $wep_b->{$k}{$sh}\n";
						}
					}
					my $mismatch = 0;
					my $in_plus_a = 0;
					my $in_plus_b = 0;
					for (my $i = 0; $i < @sha; ++$i) {
						if ($in_plus_a) {
							$mismatch = 1 if ($shb[$i] != 0);
							$in_plus_a += $sha[$i];
						} elsif ($in_plus_b) {
							$mismatch = 1 if ($sha[$i] != 0);
							$in_plus_b += $shb[$i];
						} elsif ($sha[$i] > $shb[$i]) {
							$in_plus_a = $sha[$i] - $shb[$i];
						} elsif ($sha[$i] < $shb[$i]) {
							$in_plus_b = $shb[$i] - $sha[$i];
						}
					}
					$mismatch = 1 if ($sha_plus - $in_plus_a != $shb_plus - $in_plus_b);
					if ($mismatch) {
						print STDERR "$names: $k mismatch: [" . (join ',', @sha) .
							"](+$sha_plus) != [" . (join ',', @shb) . "](+$shb_plus)\n";
					}
					$ret->{$k} = $sha_plus >= $shb_plus ? $wep_a->{$k} : $wep_b->{$k};
				}
			} elsif ($k eq "index") {
				$ret->{$k} = $wep_a->{$k} < $wep_b->{$k} ? $wep_a->{$k} : $wep_b->{$k};
			} elsif ($k eq "file_name") {
				$ret->{$k} = "*";
			} else {
				if ($wep_a->{$k} ne $wep_b->{$k}) {
					print STDERR "$names: $k mismatch: $wep_a->{$k} != $wep_b->{$k}\n";
				}
				$ret->{$k} = $wep_a->{$k};
			}
		} else {
			$ret->{$k} = $wep_a->{$k};
		}
	}
	for my $k (keys %{$wep_b}) {
		unless (defined $wep_a->{$k}) {
			$ret->{$k} = $wep_b->{$k};
		}
	}
	return $ret;
}

for my $name (@ARGV) {
	my $p = new XML::Parser(Handlers => {
		Start => \&handle_start,
		End => \&handle_end,
		Char => \&handle_char });
	@xml_string_stack = ();
	@xml_stack = ("null", "null", "null");
	$cur_file_name = $name;
	$p->parsefile($name);
}

for my $weapon (@weapons) {
	for my $k (keys %{$weapon}) {
		if ($k =~ /^name/) {
			(my $typo_k = $k) =~ s/^name/typo/;
			if (defined $translation_map{$typo_k} &&
			    defined $translation_map{$typo_k}->{$weapon->{$k}}) {
				print STDERR "typo: $weapon->{$k} -> $translation_map{$typo_k}->{$weapon->{$k}}{$k}\n";
				$weapon->{$k} = $translation_map{$typo_k}->{$weapon->{$k}}{$k}
			}
		}
	}
}

my %groups = ();
for my $weapon (@weapons) {
	if (defined $weapon->{"name"}) {
		$groups{$weapon->{"name"}} ||= [];
		push @{$groups{$weapon->{"name"}}}, $weapon;
	} else {
		my $found = 0;
		for my $k (keys %translation_map) {
			if (defined $weapon->{$k} &&
			    defined $translation_map{$k}->{$weapon->{$k}}{"name"}) {
				push @{$groups{$translation_map{$k}->{$weapon->{$k}}{"name"}}}, $weapon;
				$found = 1;
				last;
			}
		}
		unless ($found) {
			for my $k (keys %{$weapon}) {
				if ($k =~ /^name/) {
					push @{$groups{$weapon->{$k}}}, $weapon;
					$found = 1;
					last;
				}
			}
		}
	}
}

my %grouped_weapons = ();
for my $k (keys %groups) {
	if (@{$groups{$k}} < 2) {
		delete $groups{$k};
	} else {
		for my $weapon (@{$groups{$k}}) {
			$grouped_weapons{$weapon} = 1
		}
	}
}

for my $weapon (@weapons) {
	next if $grouped_weapons{$weapon};
	my $signature =
		$weapon->{"weapon_type_ref"} . '/' . ($weapon->{"attack"} || "0") . '/' .
		($weapon->{"affinity"} || "0") . '/' . ($weapon->{"affinity"} || "0");
	for my $e (sort keys %{$weapon->{"elements"}}) {
		$signature .= '/' . $e . $weapon->{"elements"}{$e} unless ($e =~ /inflated_/);
	}
	for my $s (@{$weapon->{"slots"}}) {
		$signature .= '/' . $s;
	}
	for my $sk ("red", "orange", "yellow", "green", "blue", "white", "purple", "plus") {
		if ($weapon->{"sharpness"}{$sk}) {
			$signature .= '/' . $weapon->{"sharpness"}{$sk};
		}
	}
	for my $ammo (@{$weapon->{"ammos"}}) {
		$signature .= '/' . $ammo->{"id"};
		$signature .= ':' . $ammo->{"capacity"} if (defined $ammo->{"capacity"})
	}
	$groups{$signature} ||= [];
	push @{$groups{$signature}}, $weapon;
}

my $unique_id = 15948341;
for my $k (keys %groups) {
	my %found_f = ();
	for my $weapon (@{$groups{$k}}) {
		if ($found_f{$weapon->{"file_name"}}) {
			for my $w2 (@{$groups{$k}}) {
				$groups{$unique_id++} = [ $w2 ];
			}
			delete $groups{$k};
			last;
		} else {
			$found_f{$weapon->{"file_name"}} = 1;
		}
	}
}

for my $k (keys %groups) {
	my $acc = { "index" => -1 };
	my $first = 1;
	for my $weapon (@{$groups{$k}}) {
		if ($first) {
			$acc = $weapon;
			$first = 0;
		} else {
			$acc = merge_weapons($acc, $weapon);
		}
	}
	if ($acc->{"name"} && $translation_map{"name"}{$acc->{"name"}}) {
		for my $tk (keys %{$translation_map{"name"}{$acc->{"name"}}}) {
			next unless $tk =~ /^name/;
			$acc->{$tk} = $translation_map{"name"}{$acc->{"name"}}{$tk};
		}
	} else {
		for my $ntk (keys %translation_map) {
			if (defined $acc->{$ntk} &&
			    defined $translation_map{$ntk}->{$acc->{$ntk}}) {
				for my $tk (keys %{$translation_map{$ntk}{$acc->{$ntk}}}) {
					next unless $tk =~ /^name/;
					$acc->{$tk} = $translation_map{$ntk}{$acc->{$ntk}}{$tk};
				}
				last;
			}
		}
	}
	$groups{$k} = $acc;
}

my $xml_writer;

sub print_weapon
{
	my ($weapon) = @_;

	my @ordered_names = ("name");
	for (sort keys %{$weapon}) {
		push @ordered_names, $_ if (/^name(?:_\w+)/);
	}

	my @ordered_keys =
		("weapon_type_ref", @ordered_names,
		 "inflated_attack", "attack",
		 "affinity", "awakened", "elements", "elderseal", "defense",
		 "phial", "phial_elements", "sharpness", "ammos", "notes", "slots",
		 "rare", "categories");

	$xml_writer->startTag("weapon");
	my %used_keys = ();
	for my $k (@ordered_keys) {
		next unless (defined $weapon->{$k});
		if ($k eq "phial") {
			$xml_writer->startTag("phial");
			$xml_writer->emptyTag($weapon->{$k});
			$xml_writer->endTag();
		} elsif ($k eq "elements" || $k eq "phial_elements" ) {
			if (%{$weapon->{$k}}) {
				$xml_writer->startTag($k eq "elements" ? "element" : "phial");
				for my $sk (sort {
						if ($a =~ /^inflated_(.*)/) {
							my $ao = $1;
							if ($b =~ /^inflated_(.*)/) {
								my $bo = $1;
								return $ao cmp $bo;
							} else {
								return $ao cmp $b || -1;
							}
						} else {
							if ($b =~ /^inflated_(.*)/) {
								my $bo = $1;
								return $a cmp $bo || 1;
							} else {
								return $a cmp $b;
							}
						}
					} keys %{$weapon->{$k}}) {
					$xml_writer->dataElement($sk, $weapon->{$k}{$sk})
				}
				$xml_writer->endTag();
			}
		} elsif ($k eq "sharpness") {
			if (%{$weapon->{$k}}) {
				$xml_writer->startTag("sharpness");
				for my $sk ("red", "orange", "yellow", "green", "blue", "white", "purple", "plus") {
					if ($weapon->{$k}{$sk}) {
						$xml_writer->dataElement($sk, $weapon->{$k}{$sk})
					}
				}
				$xml_writer->endTag();
			}
		} elsif ($k eq "ammos") {
			if (@{$weapon->{$k}}) {
				$xml_writer->startTag("ammos");
				for my $ammo (@{$weapon->{$k}}) {
					my $id = $ammo->{"id"};
					my $has_tag = 0;
					for my $ak (sort keys %{$ammo}) {
						next if ($ak eq "id");
						unless ($has_tag) {
							$xml_writer->startTag("ammo_ref", "id" => $id);
							$has_tag = 1;
						}
						$xml_writer->dataElement($ak, $ammo->{$ak});
					}
					if ($has_tag) {
						$xml_writer->endTag();
					} else {
						$xml_writer->emptyTag("ammo_ref", "id" => $ammo->{"id"});
					}
				}
				$xml_writer->endTag();
			}
		} elsif ($k eq "notes") {
			if (@{$weapon->{$k}}) {
				$xml_writer->startTag("notes");
				for my $s (@{$weapon->{$k}}) {
					$xml_writer->emptyTag($s);
				}
				$xml_writer->endTag();
			}
		} elsif ($k eq "slots") {
			if (@{$weapon->{$k}}) {
				$xml_writer->startTag("slots");
				for my $s (@{$weapon->{$k}}) {
					$xml_writer->dataElement("slot", $s);
				}
				$xml_writer->endTag();
			}
		} elsif ($k eq "categories") {
			if (%{$weapon->{$k}}) {
				$xml_writer->startTag("categories");
				for my $s (sort keys %{$weapon->{$k}}) {
					$xml_writer->emptyTag("category_ref", "id" => $s);
				}
				$xml_writer->endTag();
			}
		} elsif ($k eq "weapon_type_ref") {
			$xml_writer->emptyTag($k, "id" => $weapon->{$k});
		} else {
			$xml_writer->dataElement($k, $weapon->{$k});
		}
		$used_keys{$k} = 1;
	}
	for my $k (sort keys %{$weapon}) {
		unless ($used_keys{$k} || $k eq "index" || $k eq "file_name") {
			$xml_writer->dataElement($k, $weapon->{$k});
		}
	}
	$xml_writer->endTag();
}

my $io = IO::Handle->new();
$io->fdopen(fileno(STDOUT), "w");
$xml_writer = XML::Writer->new(OUTPUT => $io, ENCODING => 'utf-8', DATA_MODE => 1, DATA_INDENT => 4);
$xml_writer->xmlDecl("UTF-8");
$xml_writer->startTag("data");
for my $wkey (sort { $groups{$a}->{"index"} <=> $groups{$b}->{"index"} } (keys %groups)) {
	print_weapon($groups{$wkey});
}
$xml_writer->endTag();
