#!/usr/bin/perl

use strict;
use warnings;

use XML::Parser;
use Data::Dumper;

my @xml_stack;
my @xml_string_stack;

my @weapons;
my @monsters;
my @profiles;

my $cur_weapon = {};
my $cur_part = {};
my $cur_tolerance = {};
my $cur_monster = {};
my $cur_profile = {};

my %element_codes = (
	"fire" => 0,
	"water" => 1,
	"thunder" => 2,
	"ice" => 3,
	"dragon" => 4
);

my %status_codes = (
	"paralysis" => 0,
	"poison" => 1,
	"sleep" => 2,
	"blast" => 3,
	"exhaust" => 4
);

my %sharpness_levels = (
	"red" => 0,
	"orange" => 1,
	"yellow" => 2,
	"green" => 3,
	"blue" => 4,
	"white" => 5,
	"purple" => 6
);

sub process_start
{
	if ($xml_stack[0] eq "weapon") {
		$cur_weapon = {
			"name" => "no_name",
			"type" => "no_type",
			"attack" => 0,
			"affinity" => 0,
			"awakened" => 0,
			"elements" => {},
			"statuses" => {},
			"phial" => {},
			"sharpness" => [ 0, 0, 0, 0, 0, 0, 0 ],
			"sharpness_plus" => 0
		};
	} elsif ($xml_stack[0] eq "monster") {
		$cur_monster = {
			"name" => "no_name",
			"parts" => [],
			"tolerances" => {}
		};
		for my $tol ("poison", "sleep", "paralysis", "stun", "exhaust", "blast") {
			$cur_monster->{"tolerances"}{$tol} = {
				"init" => 0,
				"plus" => 0,
				"max" => 0,
				"regen_value" => 0,
				"regen_tick" => 0,
				"duration" => 0,
				"damage" => 0,
			}
		}
	} elsif ($xml_stack[0] eq "tolerance") {
		$cur_tolerance = {
			"type" => "no_type",
			"init" => 0,
			"plus" => 0,
			"max" => 0,
			"regen_value" => 0,
			"regen_tick" => 0,
			"duration" => 0,
			"damage" => 0,
		}
	} elsif ($xml_stack[0] eq "part") {
		$cur_part = {
			"name" => "no_name",
			"state" => "*",
			"cut" => 0,
			"impact" => 0,
			"bullet" => 0,
			"fire" => 0,
			"water" => 0,
			"thunder" => 0,
			"ice" => 0,
			"dragon" => 0,
			"stun" => 0
		}
	} elsif ($xml_stack[0] eq "weapon_profile") {
		$cur_profile = {
			"name" => "no_name",
			"type" => "no_type",
			"cut" => 0.0,
			"impact" => 0.0,
			"piercing" => 0.0,
			"sharpness_use" => 0.0,
			"element" => 0.0,
			"stun" => 0.0,
			"exhaust" => 0.0,
			"phial" => 0.0
		}
	}
}

sub process_val
{
	my $val = $xml_string_stack[0];
	$val =~ s/^\s+|\s+$//g;

	if ($xml_stack[0] eq "weapon") {
		push @weapons, $cur_weapon;
	} elsif ($xml_stack[0] eq "monster") {
		push @monsters, $cur_monster;
	} elsif ($xml_stack[0] eq "tolerance") {
		my $tol_type = $cur_tolerance->{"type"};
		delete $cur_tolerance->{"type"};
		$cur_monster->{"tolerances"}{$tol_type} = $cur_tolerance;
	} elsif ($xml_stack[0] eq "part") {
		push @{$cur_monster->{"parts"}}, $cur_part;
	} elsif ($xml_stack[0] eq "weapon_profile") {
		push @profiles, $cur_profile;
	} elsif (@xml_stack >= 2 && $xml_stack[1] eq "weapon") {
		if ($xml_stack[0] eq "name" ||
		    $xml_stack[0] eq "type" ||
		    $xml_stack[0] eq "attack" ||
		    $xml_stack[0] eq "affinity") {
			$cur_weapon->{$xml_stack[0]} = $val;
		} elsif ($xml_stack[0] eq "awakened") {
			if (lc($val) eq "true" || $val >= 1) {
				$cur_weapon->{"awakened"} = 1;
			}
		}
	} elsif (@xml_stack >= 3 &&
	    $xml_stack[2] eq "weapon" &&
	    $xml_stack[1] eq "element") {
		if (defined $element_codes{$xml_stack[0]}) {
			$cur_weapon->{"elements"}{$xml_stack[0]} = $val;
		} elsif (defined $status_codes{$xml_stack[0]}) {
			$cur_weapon->{"statuses"}{$xml_stack[0]} = $val;
		}
	} elsif (@xml_stack >= 3 &&
	    $xml_stack[2] eq "weapon" &&
	    $xml_stack[1] eq "sharpness") {
		if (defined $sharpness_levels{$xml_stack[0]}) {
			$cur_weapon->{"sharpness"}[$sharpness_levels{$xml_stack[0]}] = $val;
		}
	} elsif (@xml_stack >= 3 &&
	    $xml_stack[2] eq "weapon" &&
	    $xml_stack[1] eq "phial") {
		if ($xml_stack[0] eq "impact" ||
		    $xml_stack[0] eq "element" ||
		    $xml_stack[0] eq "power") {
			$cur_weapon->{"phial"}{$xml_stack[0]} = 1;
		} else {
			$cur_weapon->{"phial"}{$xml_stack[0]} = $val;
		}
	} elsif (@xml_stack >= 2 &&
	         $xml_stack[1] eq "monster" &&
	         $xml_stack[0] eq "name") {
		$cur_monster->{"name"} = $val;
	} elsif (@xml_stack >= 2 && $xml_stack[1] eq "part") {
		$cur_part->{$xml_stack[0]} = $val;
	} elsif (@xml_stack >= 2 && $xml_stack[1] eq "tolerance") {
		$cur_tolerance->{$xml_stack[0]} = $val;
	} elsif (@xml_stack >= 2 && $xml_stack[1] eq "weapon_profile") {
		$cur_profile->{$xml_stack[0]} = $val;
	}
}

my $string_acc = "";
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
	$string_acc = "";
	shift @xml_stack;
	shift @xml_string_stack;
}
sub handle_char
{
	my ($expat, $string) = @_;
	$xml_string_stack[0] .= $string;
}
sub parse_file
{
	my $file = $_[0];
	my $p = new XML::Parser(Handlers => {
		Start => \&handle_start,
		End => \&handle_end,
		Char => \&handle_char });
	$p->parsefile($file);
}

for my $file (@ARGV) {
	parse_file($file);
}

sub get_sharp_bonus
{
	my ($weapon, $plus, $use, $mods) = @_;

	my $wasted = $weapon->{"sharpness_plus"} - $plus;
	my @levels = @{$weapon->{"sharpness"}};

	my $i = @levels - 1;
	--$i while ($i >= 0 && $levels[$i] == 0);

	while ($wasted > 0 && $i >= 0) {
		if ($wasted > $levels[$i]) {
			$wasted -= $levels[$i];
			$levels[$i] = 0;
		} else {
			$levels[$i] -= $wasted;
			$wasted = 0;
		}
		--$i while ($i >= 0 && $levels[$i] == 0);
	}

	if ($use <= 0) {
		return ($i >= 0 ? $levels[$i] : 0);
	} else {
		my $sum = 0;
		my $remaining_use = $use;
		while ($remaining_use > 0 && $i >= 0) {
			if ($remaining_use > $levels[$i]) {
				$sum += $mods->[$i] * $levels[$i];
				$remaining_use -= $levels[$i];
				$levels[$i] = 0;
			} else {
				$sum += $mods->[$i] * $remaining_use;
				$levels[$i] -= $remaining_use;
				$remaining_use = 0;
			}
			--$i while ($i >= 0 && $levels[$i] == 0);
		}
		return $sum / $use;
	}

}

my $constants = {
	"raw_sharpness_multipiler" => [ 0.5, 0.75, 1.0, 1.05, 1.2, 1.32, 1.44 ],
	"element_sharpness_multipiler" => [ 0.25, 0.5, 0.75, 1.0, 1.0625, 1.125, 1.2 ],
	"critical_hit_multiplier" => 1.25,
	"feeble_hit_multiplier" => 0.75,
	"status_attack_rate" => (1.0 / 3.0),
	"phial_power_boost" => 1.2,
	"phial_element_boost" => 1.25
};

# don't forget powercharm/powertalon
for my $profile (@profiles) {
	for my $weapon (@weapons) {
		next if ($weapon->{"type"} ne $profile->{"type"});

		my $damage = {
			"weapon" => $weapon,
			"profile" => $profile,
			"damage" => {}
		};

		for my $state ("normal", "enraged") {
			my $state_damage = {
				"cut" => 0.0,
				"impact" => 0.0,
				"piercing" => 0.0,
				"fixed" => 0.0,
				"element" => {},
				"status" => {},
				"stun" => 0.0
			};

			my $buffed_weapon = {
				"affinity" => $weapon->{"affinity"},
				"attack" => $weapon->{"attack"},
				"elements" => $weapon->{"elements"},
				"statuses" => $weapon->{"statuses"},
				"phial_elements" => {},
				"phial_statuses" => {}
			};


			my $affinity_multiplier = 1.0;
			my $total_affinity = $buffed_weapon->{"affinity"};
			if ($total_affinity > 100) {
				$affinity_multiplier = $constants->{"critical_hit_multiplier"};
			} elsif ($total_affinity < -100) {
				$affinity_multiplier = $constants->{"feeble_hit_multiplier"};
			} elsif ($total_affinity > 0) {
				$affinity_multiplier = (1.0 - ($total_affinity / 100.0)) +
					(($total_affinity / 100.0) * $constants->{"critical_hit_multiplier"});
			} else {
				$affinity_multiplier = (1.0 + ($total_affinity / 100.0)) -
					(($total_affinity / 100.0) * $constants->{"feeble_hit_multiplier"});
			}
			my $sharp = get_sharp_bonus($weapon, 0, $profile->{"sharpness_use"},
			                            $constants->{"raw_sharpness_multipiler"});

			my $raw_attack = $buffed_weapon->{"attack"} * $affinity_multiplier * $sharp;

			if ($weapon->{"phial"}{"power"}) {
				$raw_attack *= (1.0 - $profile->{"phial_attack_rate"}) +
					($profile->{"phial_attack_rate"} * $constants->{"phial_power_boost"});
			}

			$state_damage->{"cut"} += $raw_attack * $profile->{"cut"};
			$state_damage->{"impact"} += $raw_attack * $profile->{"impact"};
			$state_damage->{"piercing"} += $raw_attack * $profile->{"piercing"};

			my $esharp = get_sharp_bonus($weapon, 0, $profile->{"sharpness_use"},
			                             $constants->{"element_sharpness_multipiler"});

			my $element_count = scalar(keys %{$buffed_weapon->{"elements"}});
			for my $element (keys %{$buffed_weapon->{"elements"}}) {
				my $element_attack = $buffed_weapon->{"elements"}{$element} * $esharp;
				if ($weapon->{"phial"}{"element"}) {
					$element_attack *= (1.0 - $profile->{"phial_element_rate"}) +
						($profile->{"phial_element_rate"} * $constants->{"phial_element_boost"});
				} elsif ($buffed_weapon->{"phial_elements"}{$element}) {
					$element_attack *= 1.0 - $profile->{"phial_element_rate"};
				}
				$state_damage->{"element"}{$element} +=
					($element_attack * $profile->{"element"}) / $element_count;
			}
			my $status_count = scalar(keys %{$buffed_weapon->{"statuses"}});
			for my $status (keys %{$buffed_weapon->{"statuses"}}) {
				my $status_attack = $buffed_weapon->{"statuses"}{$status} * $constants->{"status_attack_rate"};
				if ($buffed_weapon->{"phial_statuses"}{$status}) {
					$status_attack *= 1.0 - $profile->{"phial_element_rate"};
				}
				$state_damage->{"status"}{$status} +=
					($status_attack * $profile->{"element"}) / $status_count;
			}

			for my $element (keys %{$buffed_weapon->{"phial_elements"}}) {
				$state_damage->{"element"}{$element} +=
					$buffed_weapon->{"phial_elements"}{$element} * $esharp *
					$profile->{"phial_element_rate"} * $profile->{"element"};
			}
			for my $status (keys %{$buffed_weapon->{"phial_statuses"}}) {
				$state_damage->{"status"}{$status} +=
					$buffed_weapon->{"phial_statuses"}{$status} *
					$constants->{"status_attack_rate"} *
					$profile->{"phial_element_rate"} * $profile->{"element"};
			}

			if ($weapon->{"phial"}{"element"}) {
				for my $element (keys %{$buffed_weapon->{"elements"}}) {
					$state_damage->{"element"}{$element} +=
						($buffed_weapon->{"elements"}{$element} * $profile->{"phial_element_attack"}) / $element_count;
				}
			}

			$state_damage->{"stun"} += $profile->{"stun"};

			if ($weapon->{"phial"}{"impact"}) {
				$state_damage->{"stun"} += $profile->{"phial_impact_stun"};
				$state_damage->{"fixed"} += $weapon->{"attack"} * $profile->{"phial_impact_attack"};
			}

			$damage->{"damage"}{$state} = $state_damage;
		}
		print Dumper($damage);
	}
}


# print Dumper(@weapons);
