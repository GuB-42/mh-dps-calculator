#!/usr/bin/perl -w

use strict;
use Data::Dumper;

my %dividers = ("b"  => 1.2, "cb" => 3.6, "db" => 1.4, "gl" => 2.3, "gs" => 4.8,
                "h"  => 5.2, "hb" => 1.5, "hh" => 5.2, "ig" => 3.1, "l"  => 2.3,
                "lb" => 1.3, "ls" => 3.3, "s"  => 1.4, "sa" => 5.4);

my $sta_nb_hits = 150;
my $sta_hit_factor = 0.25;
my $sta_regen_factor = 2;

my %status_factors = (
	"poison" =>  { "duration" => 0, "damage" => 1 },
	"sleep" =>   { "base" => 450, "duration" => 0 },
	"para" =>    { "duration" => 50 },
	"blast" =>   { "damage" => 1 },
	"stun" =>    { "duration" => 50 },
	"exhaust" => { "base" => 0 }
);

my $phial_rate = 0.5;
my $phial_power_boost = 0.2;
my $phial_elt_boost = 0.25;
my $phial_ko = 7;

my $base_atk_up = 9;
my $base_atk_factor = 1.0;

my @esharp_mod = (0.25, 0.5, 0.75, 1.0, 1.0625, 1.125, 1.2);
my @sharp_mod = (0.5, 0.75, 1.0, 1.05, 1.20, 1.32, 1.44);

my %elt_id = (1 => "fire", 2 => "water", 3 => "ice", 4 => "thunder", 5 => "dragon");
my %sta_id = (6 => "poison", 7 => "para", 8 => "sleep", 9 => "blast");
my %phial_id = (1 => "power", 2 => "element", 3 => "dragon", 4 => "para", 5 => "poison", 6 => "exhaust");
my %tol_id = (1 => "poison", 2 => "sleep", 3 => "para", 4 => "stun", 5 => "exhaust", 6 => "blast");
my %shot_id = (1 => "normal", 2 => "wide", 3 => "long", 4 => "pierce", 5 => "rapid", 6 => "spread");

my %shell_neutral_dmg = (
	"normal" => [ 10, 14, 18, 21, 24 ],
	"long" => [ 15, 21, 28, 32, 36 ],
	"wide" => [ 20, 30, 40, 44, 48 ],
);
my %shell_fire_dmg = (
	"normal" => [ 4, 5, 6, 7, 8 ],
	"long" => [ 9, 11, 14, 16, 18 ],
	"wide" => [ 6, 8, 10, 11, 12 ],
);
for my $i (0 .. 4) {
	$shell_neutral_dmg{"normal"}->[$i] *= 5.5;
	$shell_neutral_dmg{"long"}->[$i] *= 3;
	$shell_neutral_dmg{"wide"}->[$i] *= 2.7;
	$shell_fire_dmg{"normal"}->[$i] *= 5.5;
	$shell_fire_dmg{"long"}->[$i] *= 3;
	$shell_fire_dmg{"wide"}->[$i] *= 2.7;
}
my @base_wyv_neutral_dmg = ( 120, 140, 160, 180, 200 );
my @base_wyv_fire_dmg = ( 40, 44, 48, 52, 56 );

my $wpn_class = $ARGV[4];

sub load_file
{
	my @out = ();
	my $record_ok = 0;
	my $record = {};
	my $record_default = 0;
	my $map = 0;
	my @header = ();
	open FILE, $_[0] || die;
	for (<FILE>) {
		chomp;
		next if /^#/;
		if (/^\s*\[(.*)\]/) {
			$map = 0;
			push @out, $record if $record_ok;
			$record_default = $record if $record_ok && $record->{"name"} eq "default";
			$record = { "name" => $1 };
			$record_ok = 1;
		} elsif (/^(\w+)\s*=\s*(.*)/) {
			$map = 0;
			if ($2 eq "%table%") {
				$map = {};
				@header = ();
				$record->{$1} = $map;
			} else {
				$record->{$1} = $2;
			}
		} elsif ($map) {
			my @values = split /\s*\|\s*/;
			if (@values) {
				if (@header) {
					$map->{$values[0]} = {};
					for my $i (0 .. (scalar @values) - 1) {
						$map->{$values[0]}{$header[$i]} = $values[$i];
					}
				} else {
					@header = @values;
				}
			} else {
				$map = 0;
			}
		}
	}
	close FILE;
	push @out, $record if $record_ok;
	$record_default = $record if $record_ok && $record->{"name"} eq "default";

	if ($record_default) {
		for $record (@out) {
			for my $key (keys %{$record_default}) {
				if (undef $record->{$key}) {
					$record->{$key} = $record_default->{$key};
				}
			}
		}
	}

	return @out;
}

my @weapons = load_file($ARGV[0]);
for my $weapon (@weapons) {
	$weapon->{"raw"} = $weapon->{"attack"} / $dividers{$weapon->{"type"}};

	$weapon->{"elements"} = {};
	$weapon->{"statuses"} = {};
	my $edivider = 0;
	for my $special (split /\s*,\s*/, $weapon->{"special"}) {
		if ($special =~ /(\w+)\s*:\s*(\d+)/) {
			my %str_type = ( "fire" => "elements", "water" => "elements", "ice" => "elements",
			                 "thunder" => "elements", "dragon" => "elements",
			                 "poison" => "statuses", "para" => "statuses",
			                 "sleep" => "statuses", "blast" => "statuses" );
			if ($str_type{$1}) {
				$weapon->{$str_type{$1}}{$1} = $2;
				$edivider += 10;
			}
		}
	}
	for my $we (keys %{$weapon->{"elements"}}) { $weapon->{"elements"}{$we} /= $edivider; }
	for my $ws (keys %{$weapon->{"statuses"}}) { $weapon->{"statuses"}{$ws} /= $edivider; }
}

my @profiles = load_file($ARGV[3]);

my @buffs = load_file($ARGV[2]);

my @monsters = load_file($ARGV[1]);
for my $monster (@monsters) {
	my %macu = ();
	for my $part (keys %{$monster->{"part"}}) {
		for my $k (keys %{$monster->{"part"}{$part}}) {
			my %no_avg = ("key" => 1, "id" => 1, "type" => 1, "name" => 1);
			unless ($no_avg{$k}) {
				$macu{$k}[0] += $monster->{"part"}{$part}{$k};
				++$macu{$k}[1];
			}
		}
	}
	if (%macu) {
		$monster->{"part"}{"!avg"}{"key"} = "!avg";
		$monster->{"part"}{"!avg"}{"id"} = "0";
		$monster->{"part"}{"!avg"}{"type"} = "";
		$monster->{"part"}{"!avg"}{"name"} = "!avg";
		for my $k (keys %macu) {
			$monster->{"part"}{"!avg"}{$k} = $macu{$k}[0] / $macu{$k}[1];
		}
	}
}

# print Dumper(%mons);
# exit;

sub get_sta_hits
{
	my $base_dmg = $_[0];
	my $tol = $_[1];
	my $factors = $_[2];

	return (0, 0) unless ($tol->{"initial"} || $tol->{"increase"});

	my $sta_tdmg = $base_dmg;
	$sta_tdmg -= $sta_regen_factor * $tol->{"regen_value"} / $tol->{"regen_tick"} if ($tol->{"regen_tick"});
	return (0, 0) if ($sta_tdmg <= 0);
	$sta_tdmg *= $sta_nb_hits;

	my $sta_hits = 0;
	my $sta_next = $tol->{"initial"};
	while ($sta_tdmg >= $sta_next) {
		$sta_tdmg -= $sta_next;
		$sta_next += $tol->{"increase"};
		$sta_next = $tol->{"max"} if ($sta_next > $tol->{"max"});
		++$sta_hits;
	}
	$sta_hits += $sta_tdmg / $sta_next;

	my $sta_dmg = 0;
	for my $dmg_type (keys %{$factors}) {
		if ($dmg_type eq "base") {
			$sta_dmg += $factors->{$dmg_type};
		} elsif ($tol->{$dmg_type}) {
			$sta_dmg += $factors->{$dmg_type} * $tol->{$dmg_type};
		}
	}
	$sta_dmg *= $sta_hits / $sta_nb_hits;

	return ($sta_hits, $sta_dmg);
}

sub max
{
	my $max = shift;
	for (@_) { $max = $_ if ($max < $_); };
	return $max;
}

sub min
{
	my $min = shift;
	for (@_) { $min = $_ if ($min > $_); };
	return $min;
}

my @wpn_buffs = ();
my %need_awaken = (
	"element" => 1, "status" => 1, "bombardier" => 1,
	"fire" => 1, "water" => 1, "ice" => 1, "thunder" => 1, "dragon" => 1
);
my %bufflist = ();
for my $wpn (@weapons) {
	next if ($wpn->{"type"} ne $wpn_class);

	for my $profile (@profiles) {
		next if ($wpn->{"type"} ne $profile->{"type"});

		my @wsharp = (0, 0);
		my @wesharp = (0, 0);
		for my $j (0, 1) {
			my @shrp = (split /\s*,\s*/, $wpn->{$j == 0 ? "sharpness" : "sharpness_plus"});
			my $nsh = $profile->{"sharpness_use"};
			for (my $i = (scalar @shrp) - 1; $i >= 0; --$i) {
				if ($shrp[$i] >= $nsh) {
					$wsharp[$j] += $sharp_mod[$i] * $nsh;
					$wesharp[$j] += $esharp_mod[$i] * $nsh;
					$nsh = 0;
				} else {
					$wsharp[$j] += $sharp_mod[$i] * $shrp[$i];
					$wesharp[$j] += $esharp_mod[$i] * $shrp[$i];
					$nsh -= $shrp[$i];
				}
			}
			$wsharp[$j] /= $profile->{"sharpness_use"};
			$wesharp[$j] /= $profile->{"sharpness_use"};
		}

		my %buffkeys = ();
		for my $buff (@buffs) {
			my $newbuff = {};
			my $buffkey = "";
			for my $bk (sort keys $buff) {
				if ($bk eq "awaken" && !$wpn->{"awakened"}) {
					$newbuff->{$bk} = $buffs->{"default"}{$bk};
				} elsif ($need_awaken{$bk} && !$buff->{"awaken"} && $wpn->{"awakened"} && $wpn->{"phial"} ne "awaken") {
					$newbuff->{$bk} = $buffs->{"default"}{$bk};
				} elsif  ($bk eq "sharp" && $wsharp[0] == $wsharp[1]) {
					$newbuff->{$bk} = $buffs->{"default"}{$bk};
				} elsif ($bk eq "element" && !%{$wpn->{"elements"}}) {
					$newbuff->{$bk} = $buffs->{"default"}{$bk};
				} elsif ($bk eq "status" && (!%{$wpn->{"statuses"}} || join(',', keys $wpn->{"statuses"}) eq "blast")) {
					$newbuff->{$bk} = $buffs->{"default"}{$bk};
				} elsif ($bk eq "bombardier" && !$wpn->{"statuses"}{"blast"}) {
					$newbuff->{$bk} = $buffs->{"default"}{$bk};
				} elsif ($bk =~ /^(fire|water|ice|thunder|dragon)$/ && !$wpn->{"elements"}{$bk}) {
					$newbuff->{$bk} = $buffs->{"default"}{$bk};
				} else {
					$newbuff->{$bk} = $buff->{$bk};
				}
				$buffkey .= "[$bk,$newbuff->{$bk}]";
			}

			unless ($buffkeys{$buffkey}) {
				my $buff = $bufflist{$buffkey} ||= $newbuff;
				my $sharp = $buff->{"sharp"} ? $wsharp[1] : $wsharp[0];
				my $afi = max(-1, min($wpn->{"affinity"} / 100.0 + $buff->{"critical"}, 1));
				my $afi_draw = $buff->{"crit_draw"} ? max($afi + 1, 1) : $afi;

				my $raw_atk = $wpn->{"raw"} + $buff->{"atk_up"} + $base_atk_up;
				$raw_atk *= $sharp * $base_atk_factor;
				$raw_atk *= 1 + $phial_power_boost * $profile->{"phial_rate"} if ($wpn->{"phial"} eq "power");

				my $raw_atk_draw = $raw_attack;
				$raw_atk *= $afi * 0.25 + 1;
				$raw_atk_draw *= $afi_draw * 0.25 + 1;

				my $ko_atk = $profile->{"ko"} + ($wpn->{"phial"} eq "exhaust" ? $phial_ko * $phial_rate : 0);
				$ko_atk *= $sharp * $buff->{"ko"};

				my $element_atk = {};
				my $status_atk = {};
				if ($buff->{"awaken"} || !$wpn->{"awakened"} || $wpn->{"phial"} eq "awaken") {
					my $aphial = $wpn->{"phial"} eq "awaken" && !$buff->{"awaken"} ? $phial_rate : 1;

					for my $element (keys %{$wpn->{"elements"}}) {
						my $esharp = $buff->{"sharp"} ? $wesharp[1] : $wesharp[0];
						my $elt_atk = $wpn->{"elements"}{$element} * $esharp;
						$elt_atk *= $aphial * max($buff->{$element}, $buff->{"element"});
						$elt_atk *= 1 + $phial_elt_boost * $phial_rate if ($wpn->{"phial"} eq "element");
						$element_atk->{$element} = $elt_atk * $profile->{"special"};
					}
					for my $status (keys %{$wpn->{"statuses"}}) {
						my $sta_atk = ($afi * 0.25 + 1) * $aphial * $wpn->{"statuses"}{$status};
						$sta_atk *= $buff->{"status"} if ($status ne "blast");
						$sta_atk *= $buff->{"bombardier"} if ($status eq "blast");
						$status_atk->{$status} = $sta_atk * $profile->{"special"} * $sta_hit_factor;
					}
				}

				my $wpn_buff = {
					"wpn" => $wpn,
					"buff" => $buff,
					"profile" => $profile,
					"cut_atk" => $raw_atk * $profile->{"cut"},
					"impact_atk" => $raw_atk * $profile->{"impact"},
					"piercing_atk" => $raw_atk * $profile->{"piercing"},
					"element_atk" => $element_atk,
					"status_atk" => $status_atk,
					"ko_atk" => $ko_atk
				};
				push @wpn_buffs, $wpn_buff;
				$buffkeys{$buffkey} = 1;
			}
		}
	}
}

# print Dumper(%bufflist);

my %glob_wpn_scores = ();
for my $monster (@monsters) {
	my %wpn_buff_dmg = ();
	my %best_wpn_buff_hash = ();
	my @best_wpn_buff_list = ();
	my $mref_tol = $monster->{"tolerance"};
	my %wpn_scores = ();

	for my $dzone (sort keys %{$monster->{"part"}}) {
		my $mref = $monster->{"part"}{$dzone};
		my @wpn_buff_list = ();
		for my $wpn_buff (@wpn_buffs) {
			my $raw_dmg = 0;
			my $elt_dmg = 0;
			my $sta_dmg = 0;
			my $sta_hits = 0;
			my $stun_hits = 0;
			my $stun_dmg = 0;
			my $exhaust_hits = 0;
			my $exhaust_dmg = 0;

			$raw_dmg += $wpn_buff->{"cut_atk"} * $mref->{"cut"} / 100;
			$raw_dmg += $wpn_buff->{"impact_atk"} * $mref->{"impact"} / 100;
			$raw_dmg += $wpn_buff->{"piercing_atk"} * max($mref->{"cut"}, $mref->{"impact"} * 0.72) / 100;
			for my $element (keys %{$wpn_buff->{"element_atk"}}) {
				next unless ($mref->{$element});
				$elt_dmg += $wpn_buff->{"element_atk"}{$element} * $mref->{$element} / 100;
			}
			for my $status (keys %{$wpn_buff->{"status_atk"}}) {
				my $tol = $mref_tol->{$status};
				next unless ($tol);
				my ($tsta_hits, $tsta_dmg) = get_sta_hits($wpn_buff->{"status_atk"}{$status}, $tol, $status_factors{$status});
				$sta_hits += $tsta_hits;
				$sta_dmg += $tsta_dmg;
			}
			if ($wpn_buff->{"ko_atk"}) {
				my $stun_atk = $wpn_buff->{"ko_atk"} * $mref->{"stun"};
				($stun_hits, $stun_dmg) = get_sta_hits($stun_atk, $mref_tol->{"stun"}, $status_factors{"stun"});
				my $exhaust_atk = $wpn_buff->{"ko_atk"} * $mref->{"exhaust"};
				($exhaust_hits, $exhaust_dmg) = get_sta_hits($$exhaust_atk, $mref_tol->{"exhaust"}, $status_factors{"exhaust"});
			}

			my $dmg = $raw_dmg + $elt_dmg + $sta_dmg + $stun_dmg + $exhaust_dmg;

			push @wpn_buff_list, [ $dmg, $wpn_buff ];
			$wpn_buff_dmg{$wpn_buff} ||= {};
			$wpn_buff_dmg{$wpn_buff}{$dzone} = [ $dmg, $raw_dmg, $elt_dmg, $sta_dmg, $sta_hits, $stun_dmg, $stun_hits, $exhaust_dmg, $exhaust_hits ];
			my $wpn = $wpn_buff->{"wpn"};
			$wpn_scores{$wpn} = [ $wpn, $dmg, 0 ] if ($dzone eq "!avg" && (!$wpn_scores{$wpn} || $wpn_scores{$wpn}[1] < $dmg));
		}

		@wpn_buff_list = sort { $b->[0] <=> $a->[0] } @wpn_buff_list;
		my $n = $dzone eq "!avg" ? 5 : 2;
		my $ndw = $dzone eq "!avg" ? 3 : 2;
		my $cnt = 0;
		my %wpn_hash = ();
		for my $wbl (@wpn_buff_list) {
			my $wpn = $wbl->[1]{"wpn"};
			if ($wpn_hash{$wpn}) {
				if ($n > 0) {
					$best_wpn_buff_hash{$wbl->[1]} = $wbl->[1];
					--$n;
				}
			} else {
				++$cnt;
				$wpn_hash{$wpn} = 1;
				if ($ndw > 0 || $n > 0) {
					$best_wpn_buff_hash{$wbl->[1]} = $wbl->[1];
					--$n; --$ndw;
				}
			}
			if ($dzone eq "!avg") {
				$wpn_scores{$wpn}[2] = 1.0 / $cnt if (!$wpn_scores{$wpn}[2]);
			} else {
				last if ($n <= 0 && $ndw <= 0);
			}
		}
	}

	print "----- $monster->{name} -----\n";
	@best_wpn_buff_list = sort { $wpn_buff_dmg{$b}{"!avg"}[0] <=> $wpn_buff_dmg{$a}{"!avg"}[0]  } (values %best_wpn_buff_hash);
	if (@best_wpn_buff_list) {
		my $best_wpn_buff = $best_wpn_buff_list[0];
		my $best_wpn_buff_dmg = $wpn_buff_dmg{$best_wpn_buff};
		my @part_list = sort { $best_wpn_buff_dmg->{$b}[0] <=> $best_wpn_buff_dmg->{$a}[0] } (keys %{$best_wpn_buff_dmg});
		my %part_id = ();
		for my $i (0 .. (@part_list - 1)) {
			$part_id{$part_list[$i]} = $i;
			printf "%2d - %s\n", $i, $part_list[$i] if ($part_list[$i] ne "!avg")
		}
		for my $wpn_buff (@best_wpn_buff_list) {
			my $wpn = $wpn_buff->{"wpn"};
			my $buff = $wpn_buff->{"buff"};

			my $wdmg = $wpn_buff_dmg{$wpn_buff};
			printf "%6.2f (%6.2f,%6.2f,%4.1f,%6.2f,%4.1f) |", $wdmg->{"!avg"}[0],
				$wdmg->{"!avg"}[1], $wdmg->{"!avg"}[2] + $wdmg->{"!avg"}[3], $wdmg->{"!avg"}[4],
				$wdmg->{"!avg"}[5], $wdmg->{"!avg"}[6];
			my @part_list2 = sort { $wdmg->{$b}[0] <=> $wdmg->{$a}[0] } (keys %{$wdmg});
			for my $part (@part_list2) {
				next if ($part eq "!avg");
				printf " %2d:%6.2f", $part_id{$part}, $wpn_buff_dmg{$wpn_buff}{$part}[0];
			}
			printf " | %s : %s", $wpn->{"name"}, $wpn->{"special"};
			my $first_buff = 1;
			for my $bk (keys %{$buff}) {
				if ($buff->{$bk} != $base_buff{$bk}) {
					if ($first_buff) {
						print " (";
						$first_buff = 0;
					} else {
						print ", ";
					}
					 print "$bk:$buff->{$bk}";
				}
			}
			print ")" unless ($first_buff);
			print "\n";
		}
	}

	for my $w (values %wpn_scores) {
		$glob_wpn_scores{$w->[0]} ||= [ $w->[0], 0, 0 ];
		$glob_wpn_scores{$w->[0]}[1] += $w->[1];
		$glob_wpn_scores{$w->[0]}[2] += $w->[2];
	}

	print "\n";
}

print "[ TOP WEAPON ]\n";
my @sorted_wpn_scores = sort { $b->[2] <=> $a->[2] } (values %glob_wpn_scores);
for my $w (@sorted_wpn_scores) {
	printf "%5.2f %6.2f : %s : %s\n", $w->[2], $w->[1] / (@monsters), $w->[0]{"name"}, $w->[0]{"special"};
}

#print Dumper(%mons);


# The math: Bouncing happens if sharpness multiplier * weapon adjustment multiplier * damage absorb < 25
# In MH3U for example - if you use a GS with green sharpness (1.125 multiplier) to hit Brachy's horn (19 absorb) with a normal attack it's going to bounce (21.375<25).
# However if you use the same sword with charge 3 (1.3 weapon adjustment multiplier) it's not going to bounce (27.7875>25).
# The multipliers corresponding to each sharpness level: Red 0.5, orange 0.75, yellow 1.0, green 1.125, blue 1.25, white 1.3, purple 1.5
# Weapon adjustments: S&S 1.12, GS charge 1 1.1, charge 2 1.2, charge 3 1.3, LS with full spirit bar 1.12 (gl : 0.97 ?
# SnS's value is now 1.06, not 1.12, and the LS flashing spirit gauge is now 1.13, as detailed in my post above. Also, sharpness values have changed.
# The 25 value is for low rank. That value increases in high and G rank. As stated, Unite used 29 for G rank. I'm not sure if that's still true or not. )
# >45 : big splosh+slow
