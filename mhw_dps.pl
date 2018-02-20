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
my $cur_hit_data = {};
my $cur_tolerance = {};
my $cur_monster = {};
my $cur_profile = {};
my $cur_pattern = {};

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
			"phial" => "no_phial",
			"phial_elements" => {},
			"phial_statuses" => {},
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
			"hit_data" => []
		}
	} elsif ($xml_stack[0] eq "hit_data") {
		$cur_hit_data = {
			"states" => {},
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
			"sharpness_use" => 0.0,
			"patterns" => []
		}
	} elsif ($xml_stack[0] eq "pattern") {
		$cur_pattern = {
			"rate" => 1.0,
			"cut" => 0.0,
			"impact" => 0.0,
			"piercing" => 0.0,
			"element" => 0.0,
			"stun" => 0.0,
			"exhaust" => 0.0,
			"phial_impact_attack" => 0.0,
			"phial_impact_stun" => 0.0,
			"phial_element_attack" => 0.0,
			"phial_ratio" => 0.0,
			"punishing_draw_stun" => 0.0
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
	} elsif ($xml_stack[0] eq "hit_data") {
		push @{$cur_part->{"hit_data"}}, $cur_hit_data;
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
			$cur_weapon->{"phial"} = $xml_stack[0];
		} elsif (defined $element_codes{$xml_stack[0]}) {
			$cur_weapon->{"phial_elements"}{$xml_stack[0]} = $val;
		} elsif (defined $status_codes{$xml_stack[0]}) {
			$cur_weapon->{"phial_statuses"}{$xml_stack[0]} = $val;
		}
	} elsif (@xml_stack >= 2 &&
	         $xml_stack[1] eq "monster" &&
	         $xml_stack[0] eq "name") {
		$cur_monster->{"name"} = $val;
	} elsif (@xml_stack >= 2 && $xml_stack[1] eq "part" &&
	         $xml_stack[0] eq "name") {
		$cur_part->{"name"} = $val;
	} elsif (@xml_stack >= 2 && $xml_stack[1] eq "hit_data") {
		if ($xml_stack[0] eq "state") {
			$cur_hit_data->{"states"}{$val} = 1;
		} else {
			$cur_hit_data->{$xml_stack[0]} = $val;
		}
	} elsif (@xml_stack >= 2 && $xml_stack[1] eq "tolerance") {
		$cur_tolerance->{$xml_stack[0]} = $val;
	} elsif (@xml_stack >= 2 && $xml_stack[1] eq "weapon_profile") {
		if ($xml_stack[0] eq "pattern") {
			push @{$cur_profile->{"patterns"}}, $cur_pattern;
		} else {
			$cur_profile->{$xml_stack[0]} = $val;
		}
	} elsif (@xml_stack >= 2 && $xml_stack[1] eq "pattern") {
		$cur_pattern->{$xml_stack[0]} = $val;
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
	"phial_element_boost" => 1.25,
	"weakness_threshold" => 45,
	"piercing_factor" => 0.72,

	"enraged_ratio" => 0.4,
	"sharpness_use" => 15,
	"buff_ratios" => {
		"draw_attack" => 0.1,
		"airborne" => 0.1,
		"red_life" => 0.2,
		"full_life" => 0.7,
		"death_1" => 0.0,
		"death_2" => 0.0,
		"full_stamina" => 0.6,
		"sliding" => 0.1,
		"low_life" => 0.05,
	}
};

my @damages = ();

sub fold_conditions_add
{
	my ($data, $conditions) = @_;
	my $ret = 0.0;
	if (defined $data) {
		for my $key (keys %{$data}) {
			$ret += $data->{$key} * $conditions->{$key};
		}
	}
	return $ret;
}
sub fold_conditions_multiply
{
	my ($data, $conditions) = @_;
	my $ret = 1.0;
	if (defined $data) {
		for my $key (keys %{$data}) {
			$ret *= (1.0 - $conditions->{$key}) +
				($data->{$key} * $conditions->{$key});
		}
	}
	return $ret;
}
sub fold_conditions_affinity
{
	my ($data, $conditions, $start_affinity) = @_;
	my $ret = $start_affinity;
	if (defined $data) {
		for my $key (sort { $data->{$1} <=> $data->{$2} } (keys %{$data})) {
			my $add_ret = $ret + $data->{$key};
			$add_ret = 100 if ($add_ret > 100);
			$add_ret = -100 if ($add_ret < -100);
			$ret += ($add_ret - $ret) * $conditions->{$key};
		}
	}
	return $ret - $start_affinity;
}
sub fold_conditions_buff_data
{
	my ($buff_data, $conditions, $start_affinity) = @_;
	my $res = {};
	$res->{"attack_plus"} =
		fold_conditions_add($buff_data->{"attack_plus"}, $conditions);
	$res->{"attack_multiplier"} =
		fold_conditions_multiply($buff_data->{"attack_multiplier"}, $conditions);
	$res->{"affinity_plus"} =
		fold_conditions_affinity($buff_data->{"affinity_plus"}, $conditions, $start_affinity);
	$res->{"element_plus"} = {};
	if (defined $buff_data->{"element_plus"}) {
		for my $element (keys %{$buff_data->{"element_plus"}}) {
			$res->{"element_plus"}{$element} =
				fold_conditions_add($buff_data->{"element_plus"}{$element}, $conditions);
		}
	}
	$res->{"element_multiplier"} = {};
	if (defined $buff_data->{"element_multiplier"}) {
		for my $element (keys %{$buff_data->{"element_multiplier"}}) {
			$res->{"element_multiplier"}{$element} =
				fold_conditions_multiply($buff_data->{"element_multiplier"}{$element}, $conditions);
		}
	}
	$res->{"status_plus"} = {};
	if (defined $buff_data->{"status_plus"}) {
		for my $status (keys %{$buff_data->{"status_plus"}}) {
			$res->{"status_plus"}{$status} =
				fold_conditions_add($buff_data->{"status_plus"}{$status}, $conditions);
		}
	}
	$res->{"status_multiplier"} = {};
	if (defined $buff_data->{"status_multiplier"}) {
		for my $status (keys %{$buff_data->{"status_multiplier"}}) {
			$res->{"status_multiplier"}{$status} =
				fold_conditions_multiply($buff_data->{"status_multiplier"}{$status}, $conditions);
		}
	}
	$res->{"awakening"} =
		fold_conditions_add($buff_data->{"awakening"}, $conditions);
	$res->{"sharpness_plus"} =
		fold_conditions_add($buff_data->{"sharpness_plus"}, $conditions);
	$res->{"sharpness_use"} =
		fold_conditions_multiply($buff_data->{"sharpness_use"}, $conditions);
	$res->{"punishing_draw_stun"} =
		fold_conditions_add($buff_data->{"stun"}, $conditions);
	$res->{"stun_multiplier"} =
		fold_conditions_multiply($buff_data->{"stun_multiplier"}, $conditions);
	if (defined $buff_data->{"raw_critical_hit_multiplier"}) {
		$res->{"raw_critical_hit_multiplier"} =
			fold_conditions_multiply($buff_data->{"raw_critical_hit_multiplier"}, $conditions);
	} else {
		$res->{"raw_critical_hit_multiplier"} = $constants->{"critical_hit_multiplier"};
	}
	$res->{"element_critical_hit_multiplier"} =
		fold_conditions_multiply($buff_data->{"element_critical_hit_multiplier"}, $conditions);
	$res->{"status_critical_hit_multiplier"} =
		fold_conditions_multiply($buff_data->{"status_critical_hit_multiplier"}, $conditions);
	return $res;
}

sub compute_buffed_weapon
{
	my ($profile, $pattern, $weapon, $state) = @_;

	my $buff = {
#		"attack_plus" => {
#			"always" => 10,
#			"enraged" => 10
#		},
#		"attack_multiplier" => {
#			"low_life" => 1.35
#		},
#		"element_multiplier" => {
#			"fire" => { "always" => 1.1 },
#		}
	};

	my $conditions = {
		"always" => 1.0,
		"enraged" => ($state->[0] eq "enraged" ? 1.0 : 0.0),
		"weak_spot" => ($state->[1] eq "weak_spot" ? 1.0 : 0.0),
		"raw_weapon" => 0.0
	};
	foreach my $key (keys %{$constants->{"buff_ratios"}}) {
		$conditions->{$key} = $constants->{"buff_ratios"}{$key};
		$conditions->{$key} = $profile->{"${key}_ratio"} if (defined $profile->{"${key}_ratio"});
		$conditions->{$key} = $pattern->{"${key}_ratio"} if (defined $pattern->{"${key}_ratio"});
	}
	if (!%{$weapon->{"elements"}} && !%{$weapon->{"statuses"}}) {
		$conditions->{"raw_weapon"} = 1.0;
	} elsif ($weapon->{"awakened"} && (!(defined $buff->{"awakening"}) || !%{$buff->{"awakening"}})) {
		$conditions->{"raw_weapon"} = 1.0;
	}
	my $buff_data = fold_conditions_buff_data($buff, $conditions, $weapon->{"affinity"});

#	print Dumper($conditions);

	my $buffed_weapon = {
		"affinity" => $weapon->{"affinity"},
		"attack" => $weapon->{"attack"},
		"elements" => {},
		"statuses" => {},
		"phial_elements" => {},
		"phial_statuses" => {},
		"phial" => $weapon->{"phial"},
		"raw_critical_hit_multiplier" => $buff_data->{"raw_critical_hit_multiplier"},
		"element_critical_hit_multiplier" => $buff_data->{"element_critical_hit_multiplier"},
		"status_critical_hit_multiplier" => $buff_data->{"status_critical_hit_multiplier"},
		"punishing_draw_stun" => $buff_data->{"punishing_draw_stun"},
		"stun_multiplier" => $buff_data->{"stun_multiplier"}
	};

	my $sharpness_use = $profile->{"sharpness_use"};
	$sharpness_use = $profile->{"sharpness_use"} if (defined $profile->{"sharpness_use"});
	$sharpness_use = $pattern->{"sharpness_use"} if (defined $pattern->{"sharpness_use"});
	$sharpness_use *= $buff_data->{"sharpness_use"};

	$buffed_weapon->{"sharp"} =
		get_sharp_bonus($weapon, $buff_data->{"sharpness_plus"}, $sharpness_use,
		                $constants->{"raw_sharpness_multipiler"});
	$buffed_weapon->{"esharp"} =
		get_sharp_bonus($weapon, $buff_data->{"sharpness_plus"}, $sharpness_use,
		                $constants->{"element_sharpness_multipiler"});

	$buffed_weapon->{"affinity"} += $buff_data->{"affinity_plus"};
	$buffed_weapon->{"attack"} += $buff_data->{"attack_plus"};
	$buffed_weapon->{"attack"} *= $buff_data->{"attack_multiplier"};

	for my $element (keys %{$weapon->{"elements"}}) {
		my $power = $weapon->{"elements"}{$element};
		if ($weapon->{"awakened"}) {
			$power *= $buff_data->{"awakening"};
		}
		if ($power > 0) {
			if (defined $buff_data->{"element_plus"}{$element}) {
				$power += $buff_data->{"element_plus"}{$element};
			}
			if (defined $buff_data->{"element_multiplier"}{$element}) {
				$power *= $buff_data->{"element_multiplier"}{$element};
			}
			$buffed_weapon->{"elements"}{$element} = $power;
		}
	}
	for my $status (keys %{$weapon->{"statuses"}}) {
		my $power = $weapon->{"statuses"}{$status};
		if ($weapon->{"awakened"}) {
			$power *= $buff_data->{"awakening"};
		}
		if ($power > 0) {
			if (defined $buff_data->{"status_plus"}{$status}) {
				$power += $buff_data->{"status_plus"}{$status};
			}
			if (defined $buff_data->{"status_multiplier"}{$status}) {
				$power *= $buff_data->{"status_multiplier"}{$status};
			}
			$buffed_weapon->{"statuses"}{$status} = $power;
		}
	}
	for my $element (keys %{$weapon->{"phial_elements"}}) {
		my $power = $weapon->{"phial_elements"}{$element};
		if (defined $buff_data->{"element_plus"}{$element}) {
			$power += $buff_data->{"element_plus"}{$element};
		}
		if (defined $buff_data->{"element_multiplier"}{$element}) {
			$power *= $buff_data->{"element_multiplier"}{$element};
		}
		$buffed_weapon->{"phial_elements"}{$element} = $power;
	}
	for my $status (keys %{$weapon->{"phial_statuses"}}) {
		my $power = $weapon->{"phial_statuses"}{$status};
		if (defined $buff_data->{"status_plus"}{$status}) {
			$power += $buff_data->{"status_plus"}{$status};
		}
		if (defined $buff_data->{"status_multiplier"}{$status}) {
			$power *= $buff_data->{"status_multiplier"}{$status};
		}
		$buffed_weapon->{"phial_statuses"}{$status} = $power;
	}

#	print Dumper($buffed_weapon);

	return $buffed_weapon;
}

sub compute_damage
{
	my ($profile, $pattern, $buffed_weapon) = @_;

	my $damage = {
		"cut" => 0.0,
		"impact" => 0.0,
		"piercing" => 0.0,
		"fixed" => 0.0,
		"element" => {},
		"status" => {},
		"stun" => 0.0
	};

	my $raw_affinity_multiplier = 1.0;
	my $element_affinity_multiplier = 1.0;
	my $status_affinity_multiplier = 1.0;

	if ($buffed_weapon->{"affinity"} > 0) {
		$raw_affinity_multiplier = (1.0 - ($buffed_weapon->{"affinity"} / 100.0)) +
			(($buffed_weapon->{"affinity"} / 100.0) * $buffed_weapon->{"raw_critical_hit_multiplier"});
		$element_affinity_multiplier = (1.0 - ($buffed_weapon->{"affinity"} / 100.0)) +
			(($buffed_weapon->{"affinity"} / 100.0) * $buffed_weapon->{"element_critical_hit_multiplier"});
		$status_affinity_multiplier = (1.0 - ($buffed_weapon->{"affinity"} / 100.0)) +
			(($buffed_weapon->{"affinity"} / 100.0) * $buffed_weapon->{"status_critical_hit_multiplier"});
	} elsif ($buffed_weapon->{"affinity"} < 0) {
		$raw_affinity_multiplier = (1.0 + ($buffed_weapon->{"affinity"} / 100.0)) -
			(($buffed_weapon->{"affinity"} / 100.0) * $constants->{"feeble_hit_multiplier"});
	}

	my $raw_attack = $buffed_weapon->{"attack"} *
		$raw_affinity_multiplier * $buffed_weapon->{"sharp"};

	if ($buffed_weapon->{"phial"} eq "power") {
		$raw_attack *= (1.0 - $pattern->{"phial_ratio"}) +
			($pattern->{"phial_ratio"} * $constants->{"phial_power_boost"});
	}

	$damage->{"cut"} += $raw_attack * $pattern->{"cut"};
	$damage->{"impact"} += $raw_attack * $pattern->{"impact"};
	$damage->{"piercing"} += $raw_attack * $pattern->{"piercing"};

	my $element_multiplier =
		$element_affinity_multiplier * $buffed_weapon->{"esharp"};
	my $status_multiplier =
		$status_affinity_multiplier * $constants->{"status_attack_rate"};

	my $element_count = scalar(keys %{$buffed_weapon->{"elements"}});
	for my $element (keys %{$buffed_weapon->{"elements"}}) {
		my $element_attack = $buffed_weapon->{"elements"}{$element} * $element_multiplier;
		if ($buffed_weapon->{"phial"} eq "element") {
			$element_attack *= (1.0 - $pattern->{"phial_ratio"}) +
				($pattern->{"phial_ratio"} * $constants->{"phial_element_boost"});
		} elsif ($buffed_weapon->{"phial_elements"}{$element}) {
			$element_attack *= 1.0 - $pattern->{"phial_ratio"};
		}
		$damage->{"elements"}{$element} +=
			($element_attack * $pattern->{"element"}) / $element_count;
	}
	my $status_count = scalar(keys %{$buffed_weapon->{"statuses"}});
	for my $status (keys %{$buffed_weapon->{"statuses"}}) {
		my $status_attack = $buffed_weapon->{"statuses"}{$status} * $status_multiplier;
		if ($buffed_weapon->{"phial_statuses"}{$status}) {
			$status_attack *= 1.0 - $pattern->{"phial_ratio"};
		}
		$damage->{"statuses"}{$status} +=
			($status_attack * $pattern->{"element"}) / $status_count;
	}

	for my $element (keys %{$buffed_weapon->{"phial_elements"}}) {
		$damage->{"elements"}{$element} +=
			$buffed_weapon->{"phial_elements"}{$element} * $element_multiplier *
			$pattern->{"phial_ratio"} * $pattern->{"element"};
	}
	for my $status (keys %{$buffed_weapon->{"phial_statuses"}}) {
		$damage->{"statuses"}{$status} +=
			$buffed_weapon->{"phial_statuses"}{$status} * $status_multiplier *
			$pattern->{"phial_ratio"} * $pattern->{"element"};
	}

	if ($buffed_weapon->{"phial"} eq "element") {
		for my $element (keys %{$buffed_weapon->{"elements"}}) {
			$damage->{"elements"}{$element} +=
				($buffed_weapon->{"elements"}{$element} * $pattern->{"phial_element_attack"}) / $element_count;
		}
	}

	$damage->{"stun"} += $pattern->{"stun"};
	$damage->{"stun"} += $pattern->{"punishing_draw_stun"} * $buffed_weapon->{"punishing_draw_stun"};

	if ($buffed_weapon->{"phial"} eq "impact") {
		$damage->{"stun"} += $pattern->{"phial_impact_stun"};
		$damage->{"fixed"} += $buffed_weapon->{"attack"} * $pattern->{"phial_impact_attack"};
	}

	$damage->{"stun"} *= $buffed_weapon->{"stun_multiplier"};

#	print Dumper($damage);

	return $damage;
}

# don't forget powercharm/powertalon
for my $profile (@profiles) {
	for my $weapon (@weapons) {
		next if ($weapon->{"type"} ne $profile->{"type"});

		my $damage = {
			"weapon" => $weapon,
			"profile" => $profile,
			"damage" => {}
		};

		for my $state ([ "!enraged", "normal" ],
		               [ "enraged", "normal" ],
		               [ "!enraged", "weak_point" ],
		               [ "enraged", "weak_point" ]) {
			for my $pattern (@{$profile->{"patterns"}}) {
				my $buffed_weapon = compute_buffed_weapon($profile, $pattern, $weapon, $state);
				my $state_damage = compute_damage($profile, $pattern, $buffed_weapon);

				$damage->{"damage"}{$state->[0]} ||= {};
				$damage->{"damage"}{$state->[0]}{$state->[1]} ||= {};
				my $total_state_damage = $damage->{"damage"}{$state->[0]}{$state->[1]};

				$total_state_damage->{"cut"} ||= 0.0;
				$total_state_damage->{"cut"} += $state_damage->{"cut"} * $pattern->{"rate"};
				$total_state_damage->{"impact"} ||= 0.0;
				$total_state_damage->{"impact"} += $state_damage->{"impact"} * $pattern->{"rate"};
				$total_state_damage->{"piercing"} ||= 0.0;
				$total_state_damage->{"piercing"} += $state_damage->{"piercing"} * $pattern->{"rate"};
				$total_state_damage->{"fixed"} ||= 0.0;
				$total_state_damage->{"fixed"} += $state_damage->{"fixed"} * $pattern->{"rate"};
				$total_state_damage->{"stun"} ||= 0.0;
				$total_state_damage->{"stun"} += $state_damage->{"stun"} * $pattern->{"rate"};
				$total_state_damage->{"elements"} ||= {};
				for my $element (keys %{$state_damage->{"elements"}}) {
					$total_state_damage->{"elements"}{$element} ||= 0.0;
					$total_state_damage->{"elements"}{$element} +=
						$state_damage->{"elements"}{$element} *= $pattern->{"rate"};
				}
				$total_state_damage->{"statuses"} ||= {};
				for my $status (keys %{$state_damage->{"statuses"}}) {
					$total_state_damage->{"statuses"}{$status} ||= 0.0;
					$total_state_damage->{"statuses"}{$status} +=
						$state_damage->{"statuses"}{$status} *= $pattern->{"rate"};
				}
			}
		}
		push @damages, $damage;
	}
}

for my $monster (@monsters) {
	for my $part (@{$monster->{"parts"}}) {
		for my $damage (@damages) {
			my $dps = {};

			for my $enraged_state ("!enraged", "enraged") {
				my $not_enraged_state = "!$enraged_state";
				$not_enraged_state =~ s/^!!//;
				my $nb_hit_data = 0;
				for my $hit_data (@{$part->{"hit_data"}}) {
					next if ($hit_data->{"states"}{$not_enraged_state});
					++$nb_hit_data;
				}
				for my $hit_data (@{$part->{"hit_data"}}) {
					next if ($hit_data->{"states"}{$not_enraged_state});
					my $state_dps = { "raw" => 0.0, "element" => 0.0, "status" => 0.0 } ;
					my $state_damage_normal = $damage->{"damage"}{$enraged_state}{"normal"};
					my $state_damage_weak = $damage->{"damage"}{$enraged_state}{"weak_point"};

					my %hit_types = ("cut" => $hit_data->{"cut"}, "impact" => $hit_data->{"impact"});
					$hit_types{"piercing"} = $hit_data->{"impact"} * $constants->{"piercing_factor"};
					$hit_types{"piercing"} = $hit_data->{"cut"} if $hit_types{"piercing"} < $hit_data->{"cut"};
					my $weak_sum = 0.0;
					my $weak_divider = 0.0;
					for my $hit_type (keys %hit_types) {
						if ($hit_types{$hit_type} >= $constants->{"weakness_threshold"}) {
							$weak_sum += $state_damage_weak->{$hit_type};
							$weak_divider += $state_damage_weak->{$hit_type};
							$state_dps->{"raw"} += $state_damage_weak->{$hit_type} * $hit_types{$hit_type} / 100.0;
						} else {
							$weak_divider += $state_damage_normal->{$hit_type};
							$state_dps->{"raw"} += $state_damage_normal->{$hit_type} * $hit_types{$hit_type} / 100.0;
						}
					}
					my $weak_ratio = $weak_divider > 0.0 ? ($weak_sum / $weak_divider) : 0.0;
					for my $element (keys %{$state_damage_weak->{"elements"}}) {
						$state_dps->{"element"} += $state_damage_weak->{"elements"}{$element} *
							$hit_data->{$element} * $weak_ratio / 100.0;
					}
					for my $element (keys %{$state_damage_normal->{"elements"}}) {
						$state_dps->{"element"} += $state_damage_normal->{"elements"}{$element} *
							$hit_data->{$element} * (1.0 - $weak_ratio) / 100.0;
					}

					$state_dps->{"total"} = $state_dps->{"raw"} + $state_dps->{"element"} + $state_dps->{"status"};

					my $ratio = $constants->{"enraged_ratio"};
					$ratio = 1.0 - $ratio if ($enraged_state eq "!enraged");
					$ratio /= $nb_hit_data;
					for my $key (keys %{$state_dps}) {
						$dps->{$key} += $state_dps->{$key} * $ratio;
					}
				}
			}
			printf "%6.2f :: %6.2f :: %6.2f :: %s / %s / %s / %s\n",
				($dps->{total}, $dps->{raw}, $dps->{element}, $monster->{name}, $part->{name}, $damage->{weapon}{name}, $damage->{profile}{name});
		#			print "$dps->{total} :: $dps->{raw} :: $dps->{element} :: $monster->{name} / $part->{name} / $damage->{weapon}{name} / $damage->{profile}{name}\n";
		}
	}
}

#print Dumper(@monsters);
