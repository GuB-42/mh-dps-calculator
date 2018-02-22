#!/usr/bin/perl

use strict;
use warnings;

use FindBin;
use lib $FindBin::Bin;
use DataReader;

use Data::Dumper;
$Data::Dumper::Sortkeys = 1;

# time is in seconds

# TODO
# maximum element buff = +20%
# elemental crit adjustment (M:1.2/L:?/XL:1.35)

my $constants = {
	"raw_sharpness_multipilers" => [ 0.5, 0.75, 1.0, 1.05, 1.2, 1.32, 1.44 ],
	"element_sharpness_multipilers" => [ 0.25, 0.5, 0.75, 1.0, 1.0625, 1.125, 1.2 ],
	"raw_critical_hit_multiplier" => 1.25,
	"element_critical_hit_multiplier" => 1.0,
	"status_critical_hit_multiplier" => 1.0,
	"feeble_hit_multiplier" => 0.75,
	"status_attack_rate" => (1.0 / 3.0),
	"phial_power_boost" => 1.2,
	"phial_element_boost" => 1.25,
	"weakness_threshold" => 45,
	"element_weakness_threshold" => 20,
	"piercing_factor" => 0.72,
	"bounce_threshold" => 25,

	"enraged_ratio" => 0.4,
	"sharpness_use" => 15,
	"sharpen_period" => 300.0,
	"monster_hit_points" => 6000.0,

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
		"received_damage" => 0.5,
		"might_seed_use" => 0.5,
		"demon_powder_use" => 0.5,
		"might_pill_use" => 0.1
	}
};

sub get_sharp_bonus_array
{
	my ($weapon, $plus, $use, $max_ratio, $mods) = @_;

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

	my $max_bonus = ($i >= 0 ? $mods->[$i] : 0);
	if ($i < 0) {
		return [ [1.0, ($mods->[0] || 0.0)] ];
	} elsif ($use <= 0 || $max_ratio >= 1.0 || $i < 0) {
		return [ [1.0, $mods->[$i]] ];
	} else {
		my $res = [];
		my $remaining_use = $use * (1.0 - $max_ratio);
		while ($remaining_use > 0 && $i >= 0) {
			if ($remaining_use > $levels[$i]) {
				push @{$res}, [$levels[$i] / $use, $mods->[$i]];
				$remaining_use -= $levels[$i];
				$levels[$i] = 0;
			} else {
				push @{$res}, [$remaining_use / $use, $mods->[$i]];
				$levels[$i] -= $remaining_use;
				$remaining_use = 0;
			}
			--$i while ($i >= 0 && $levels[$i] == 0);
		}
		$res->[0][0] += $max_ratio;
		return $res;
	}
}
sub get_sharp_bonus
{
	my $ret = 0.0;
	my $a = get_sharp_bonus_array(@_);
	for my $v (@{$a}) {
		$ret += $v->[0] * $v->[1];
	}
	return $ret;
}

sub buff_combine_add
{
	my ($acu, $value, $ratio, $min, $max) = @_;
	$acu = 0.0 if (!defined $acu);
	$value = 0.0 if (!defined $value);
	$ratio = 1.0 if (!defined $ratio);
	my $add_ret = $acu + $value;
	$add_ret = $max if (defined $max && $add_ret > $max);
	$add_ret = $min if (defined $min && $add_ret < $min);
	return $acu + ($add_ret - $acu) * $ratio;
}
sub buff_combine_multiply
{
	my ($acu, $value, $ratio, $min, $max) = @_;
	$acu = 1.0 if (!defined $acu);
	$value = 1.0 if (!defined $value);
	$ratio = 1.0 if (!defined $ratio);
	my $mult_ret = $acu * $value;
	$mult_ret = $max if (defined $max && $mult_ret > $max);
	$mult_ret = $min if (defined $min && $mult_ret < $min);
	return $acu + ($mult_ret - $acu) * $ratio;
}
sub buff_combine_max
{
	my ($acu, $value, $ratio) = @_;
	$acu = 0.0 if (!defined $acu);
	$value = 0.0 if (!defined $value);
	$ratio = 1.0 if (!defined $ratio);
	my $max_ret = $acu > $value ? $acu : $value;
	return $acu + ($max_ret - $acu) * $ratio;
}
my %buff_combiners = (
	"attack_plus" => \&buff_combine_add,
	"attack_multiplier" => \&buff_combine_multiply,
	"affinity_plus" => \&buff_combine_add,
	"element_plus" => \&buff_combine_add,
	"element_multiplier" => \&buff_combine_multiply,
	"status_plus" => \&buff_combine_add,
	"status_multiplier" => \&buff_combine_multiply,
	"all_elements_plus" => \&buff_combine_add,
	"all_elements_multiplier" => \&buff_combine_multiply,
	"all_statuses_plus" => \&buff_combine_add,
	"all_statuses_multiplier" => \&buff_combine_multiply,
	"awakening" => \&buff_combine_add,
	"sharpness_plus" => \&buff_combine_add,
	"max_sharpness_time" => \&buff_combine_add,
	"sharpness_use_multiplier" => \&buff_combine_multiply,
	"sharpness_use_critical_multiplier" => \&buff_combine_multiply,
	"draw_attack_stun" => \&buff_combine_add,
	"draw_attack_exhaust" => \&buff_combine_add,
	"stun_multiplier" => \&buff_combine_multiply,
	"exhaust_multiplier" => \&buff_combine_multiply,
	"artillery_multiplier" => \&buff_combine_multiply,
	"raw_critical_hit_multiplier" => \&buff_combine_max,
	"element_critical_hit_multiplier" => \&buff_combine_max,
	"status_critical_hit_multiplier" => \&buff_combine_max,
	"minds_eye" => \&buff_combine_add
);
sub fold_conditions
{
	my ($start, $data, $conditions, $combiner, $min, $max) = @_;
	my $ret = $start;
	if (defined $data) {
		for my $key (sort { $data->{$a} <=> $data->{$b} } (keys %{$data})) {
			$ret = $combiner->($ret, $data->{$key}, $conditions->{$key}, $min, $max);
		}
	}
	return defined $ret ? $ret : $combiner->();
}
sub fold_conditions_buff_data
{
	my ($buff_data, $conditions, $start_affinity) = @_;
	my $res = {
		"element_plus" => {},
		"element_multiplier" => {},
		"status_plus" => {},
		"status_multiplier" => {},
		"raw_critical_hit_multiplier" => $constants->{"raw_critical_hit_multiplier"},
		"element_critical_hit_multiplier" => $constants->{"element_critical_hit_multiplier"},
		"status_critical_hit_multiplier" => $constants->{"status_critical_hit_multiplier"}
	};
	for my $buff_key (keys %buff_combiners) {
		if ($buff_key eq "affinity_plus") {
			$res->{$buff_key} =
				fold_conditions($res->{$buff_key},
				                $buff_data->{$buff_key},
				                $conditions,
				                $buff_combiners{$buff_key},
				                -100 - $start_affinity,
				                100 - $start_affinity);
		} elsif ($buff_key =~ /^(element|status)_(plus|multiplier)$/) {
			for my $elt_key (keys %{$buff_data->{$buff_key}}) {
				$res->{$buff_key}{$elt_key} =
					fold_conditions($res->{$buff_key}{$elt_key},
					                $buff_data->{$buff_key}{$elt_key},
					                $conditions,
					                $buff_combiners{$buff_key});
			}
		} else {
			$res->{$buff_key} =
				fold_conditions($res->{$buff_key},
				                $buff_data->{$buff_key},
				                $conditions,
				                $buff_combiners{$buff_key});
		}
	}
	return $res;
}
sub all_buff_combine {
	my ($acu, $other) = @_;
	for my $buff_key (keys %{$other}) {
		$acu->{$buff_key} ||= {};
		if ($buff_key =~ /^(element|status)_(plus|multiplier)$/) {
			for my $elt_key (keys %{$other->{$buff_key}}) {
				$acu->{$buff_key}{$elt_key} ||= {};
				for my $condition_key (keys %{$other->{$buff_key}{$elt_key}}) {
					$acu->{$buff_key}{$elt_key}{$condition_key} =
						buff_combiners{$buff_key}->($acu->{$buff_key}{$elt_key}{$condition_key},
					                                $other->{$buff_key}{$elt_key}{$condition_key});
				}
			}
		} else {
			for my $condition_key (keys %{$other->{$buff_key}}) {
				$acu->{$buff_key}{$condition_key} =
					buff_combiners{$buff_key}->($acu->{$buff_key}{$condition_key},
				                                $other->{$buff_key}{$condition_key});
			}
		}
	}
}


sub compute_buffed_weapon
{
	my ($profile, $pattern, $weapon, $state, $buff) = @_;

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
		"draw_attack_stun" => $buff_data->{"draw_attack_stun"},
		"draw_attack_exhaust" => $buff_data->{"draw_attack_exhaust"},
		"stun_multiplier" => $buff_data->{"stun_multiplier"},
		"exhaust_multiplier" => $buff_data->{"exhaust_multiplier"},
		"artillery_multiplier" => $buff_data->{"artillery_multiplier"},
		"bounce_sharpness" => []
	};

	$buffed_weapon->{"affinity"} += $buff_data->{"affinity_plus"};
	$buffed_weapon->{"attack"} += $buff_data->{"attack_plus"};
	$buffed_weapon->{"attack"} *= $buff_data->{"attack_multiplier"};

	my $sharpness_use = $profile->{"sharpness_use"};
	$sharpness_use = $profile->{"sharpness_use"} if (defined $profile->{"sharpness_use"});
	$sharpness_use = $pattern->{"sharpness_use"} if (defined $pattern->{"sharpness_use"});
	$sharpness_use *= $buff_data->{"sharpness_use_multiplier"};
	if ($buffed_weapon->{"affinity"} > 0) {
		$sharpness_use +=
			($buff_data->{"sharpness_use_critical_multiplier"} - 1.0) *
			$sharpness_use * $buffed_weapon->{"affinity"} / 100.0;
	}

	my $sharpen_period = $profile->{"sharpen_period"};
	$sharpen_period = $profile->{"sharpen_period"} if (defined $profile->{"sharpen_period"});
	$sharpen_period = $pattern->{"sharpen_period"} if (defined $pattern->{"sharpen_period"});

	$buffed_weapon->{"sharp"} =
		get_sharp_bonus($weapon, $buff_data->{"sharpness_plus"}, $sharpness_use,
		                $buff_data->{"max_sharpness_time"} / $constants->{"sharpen_period"},
		                $constants->{"raw_sharpness_multipilers"});
	$buffed_weapon->{"esharp"} =
		get_sharp_bonus($weapon, $buff_data->{"sharpness_plus"}, $sharpness_use,
		                $buff_data->{"max_sharpness_time"} / $constants->{"sharpen_period"},
		                $constants->{"element_sharpness_multipilers"});

	if ($buff_data->{"minds_eye"} < 1.0) {
		$buffed_weapon->{"bounce_sharpness"} =
			get_sharp_bonus_array($weapon, $buff_data->{"sharpness_plus"}, $sharpness_use,
			                      $buff_data->{"max_sharpness_time"} / $constants->{"sharpen_period"},
			                      $constants->{"raw_sharpness_multipilers"});
		if ($buff_data->{"minds_eye"} > 0.0) {
			for my $v (@{$buffed_weapon->{"bounce_sharpness"}}) {
				$v->[1] *= 1.0 - $buff_data->{"minds_eye"};
			}
			push @{$buffed_weapon->{"bounce_sharpness"}}, [$buff_data->{"minds_eye"}, 999];
		}
	} else {
		push @{$buffed_weapon->{"bounce_sharpness"}}, [1.0, 999];
	}

	my $multi_divider =
		scalar(%{$weapon->{"elements"}}) + scalar(%{$weapon->{"statuses"}});

	for my $element (keys %{$weapon->{"elements"}}) {
		my $power = $weapon->{"elements"}{$element} / $multi_divider;
		if ($weapon->{"awakened"}) {
			$power *= $buff_data->{"awakening"};
		}
		if ($power > 0) {
			$power += $buff_data->{"all_elements_plus"};
			if (defined $buff_data->{"element_plus"}{$element}) {
				$power += $buff_data->{"element_plus"}{$element};
			}
			$power *= $buff_data->{"all_elements_multiplier"};
			if (defined $buff_data->{"element_multiplier"}{$element}) {
				$power *= $buff_data->{"element_multiplier"}{$element};
			}
			$buffed_weapon->{"elements"}{$element} = $power;
		}
	}
	for my $status (keys %{$weapon->{"statuses"}}) {
		my $power = $weapon->{"statuses"}{$status} / $multi_divider;
		if ($weapon->{"awakened"}) {
			$power *= $buff_data->{"awakening"};
		}
		if ($power > 0) {
			$power += $buff_data->{"all_statuses_plus"};
			if (defined $buff_data->{"status_plus"}{$status}) {
				$power += $buff_data->{"status_plus"}{$status};
			}
			$power += $buff_data->{"all_statuses_multiplier"};
			if (defined $buff_data->{"status_multiplier"}{$status}) {
				$power *= $buff_data->{"status_multiplier"}{$status};
			}
			$buffed_weapon->{"statuses"}{$status} = $power;
		}
	}
	for my $element (keys %{$weapon->{"phial_elements"}}) {
		my $power = $weapon->{"phial_elements"}{$element} / $multi_divider;
		$power += $buff_data->{"all_elements_plus"};
		if (defined $buff_data->{"element_plus"}{$element}) {
			$power += $buff_data->{"element_plus"}{$element};
		}
		$power *= $buff_data->{"all_elements_multiplier"};
		if (defined $buff_data->{"element_multiplier"}{$element}) {
			$power *= $buff_data->{"element_multiplier"}{$element};
		}
		$buffed_weapon->{"phial_elements"}{$element} = $power;
	}
	for my $status (keys %{$weapon->{"phial_statuses"}}) {
		my $power = $weapon->{"phial_statuses"}{$status} / $multi_divider;
		$power += $buff_data->{"all_statuses_plus"};
		if (defined $buff_data->{"status_plus"}{$status}) {
			$power += $buff_data->{"status_plus"}{$status};
		}
		$power += $buff_data->{"all_statuses_multiplier"};
		if (defined $buff_data->{"status_multiplier"}{$status}) {
			$power *= $buff_data->{"status_multiplier"}{$status};
		}
		$buffed_weapon->{"phial_statuses"}{$status} = $power;
	}

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
		"elements" => {},
		"statuses" => {},
		"bounce_sharpness" => [],
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
		$raw_affinity_multiplier * $buffed_weapon->{"sharp"} * $pattern->{"sharpness_multiplier"};

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

	for my $element (keys %{$buffed_weapon->{"elements"}}) {
		my $element_attack = $buffed_weapon->{"elements"}{$element} * $element_multiplier;
		if ($buffed_weapon->{"phial"} eq "element") {
			$element_attack *= (1.0 - $pattern->{"phial_ratio"}) +
				($pattern->{"phial_ratio"} * $constants->{"phial_element_boost"});
		} elsif ($buffed_weapon->{"phial_elements"}{$element}) {
			$element_attack *= 1.0 - $pattern->{"phial_ratio"};
		}
		$damage->{"elements"}{$element} += $element_attack * $pattern->{"element"};
	}
	for my $status (keys %{$buffed_weapon->{"statuses"}}) {
		my $status_attack = $buffed_weapon->{"statuses"}{$status} * $status_multiplier;
		if ($buffed_weapon->{"phial_statuses"}{$status}) {
			$status_attack *= 1.0 - $pattern->{"phial_ratio"};
		}
		$damage->{"statuses"}{$status} += $status_attack * $pattern->{"status"};
	}

	for my $element (keys %{$buffed_weapon->{"phial_elements"}}) {
		$damage->{"elements"}{$element} +=
			$buffed_weapon->{"phial_elements"}{$element} * $element_multiplier *
			$pattern->{"phial_ratio"} * $pattern->{"element"};
	}
	for my $status (keys %{$buffed_weapon->{"phial_statuses"}}) {
		$damage->{"statuses"}{$status} +=
			$buffed_weapon->{"phial_statuses"}{$status} * $status_multiplier *
			$pattern->{"phial_ratio"} * $pattern->{"status"};
	}

	if ($buffed_weapon->{"phial"} eq "element") {
		for my $element (keys %{$buffed_weapon->{"elements"}}) {
			$damage->{"elements"}{$element} +=
				$buffed_weapon->{"elements"}{$element} * $pattern->{"phial_element_attack"};
		}
	}

	my $stun = $damage->{"statuses"}{"stun"} || 0.0;
	my $exhaust = $damage->{"statuses"}{"exhaust"} || 0.0;

	$stun += $pattern->{"stun"};
	$stun += $pattern->{"draw_attack"} * $buffed_weapon->{"draw_attack_stun"};
	$exhaust += $pattern->{"exhaust"};
	$exhaust += $pattern->{"draw_attack"} * $buffed_weapon->{"draw_attack_exhaust"};

	if ($buffed_weapon->{"phial"} eq "impact") {
		$stun += $pattern->{"phial_impact_stun"};
		$exhaust += $pattern->{"phial_impact_exhaust"};
		$damage->{"fixed"} += $buffed_weapon->{"attack"} * $buffed_weapon->{"artillery_multiplier"} *
			$pattern->{"phial_impact_attack"};
	}

	$stun *= $buffed_weapon->{"stun_multiplier"};
	$exhaust *= $buffed_weapon->{"exhaust_multiplier"};

	for my $v (@{$buffed_weapon->{"bounce_sharpness"}}) {
		push @{$damage->{"bounce_sharpness"}},
			[$v->[0], $v->[1] * $pattern->{"sharpness_multiplier"}];
	}

	$damage->{"statuses"}{"exhaust"} += $exhaust if ($exhaust > 0.0);
	$damage->{"statuses"}{"stun"} += $stun if ($stun > 0.0);

	return $damage;
}

sub get_status_hits
{
	my ($status_attack, $tolerance, $period, $overbuild) = @_;

	if ($tolerance->{"regen_value"} > 0.0) {
		$status_attack -=
			$tolerance->{"regen_value"} / $tolerance->{"regen_tick"};
	}
	return 0 if ($status_attack <= 0);
	my $duration = 0.0;
	my $hits = 0.0;
	for (1..3) {
		$hits = 0.0;
		my $dmg = $status_attack * ($period - $duration);
		my $tol = $tolerance->{"initial"};
		while ($tol < $tolerance->{"max"} && $dmg > 0) {
			if ($dmg > $tol) {
				$dmg -= $tol;
				$tol += $tolerance->{"plus"};
				$hits += 1.0;
			} else {
				$hits += $dmg / $tol;
				$dmg = 0;
			}
		}
		$hits += $dmg / $tolerance->{"max"} if $tolerance->{"max"} > 0;
		$duration = $hits * $tolerance->{"duration"};
		last unless ($overbuild && $duration > 0.0);
	}
	return $hits / $period;
}

my $data = {
	"buffs" => [ { "name" => "No buff", "data" => {} } ]
};
parse_data_files($data, @ARGV);

my @damages = ();

for my $profile (@{$data->{"profiles"}}) {
	for my $weapon (@{$data->{"weapons"}}) {
		next if ($weapon->{"type"} ne $profile->{"type"});
		for my $buff (@{$data->{"buffs"}}) {

			my $damage = {
				"weapon" => $weapon,
				"profile" => $profile,
				"buff" => $buff,
				"damage" => {}
			};

			for my $state ([ "!enraged", "normal" ],
			               [ "enraged", "normal" ],
			               [ "!enraged", "weak_spot" ],
			               [ "enraged", "weak_spot" ]) {
				my $total_rate = 0.0;
				for my $pattern (@{$profile->{"patterns"}}) {
					$total_rate += $pattern->{"rate"} / $pattern->{"period"};
				}
				for my $pattern (@{$profile->{"patterns"}}) {
					my $rate = $pattern->{"rate"} / $pattern->{"period"};
					my $buffed_weapon = compute_buffed_weapon($profile, $pattern, $weapon, $state, $buff->{"data"});
					my $state_damage = compute_damage($profile, $pattern, $buffed_weapon);

					$damage->{"damage"}{$state->[0]} ||= {};
					$damage->{"damage"}{$state->[0]}{$state->[1]} ||= {};
					my $total_state_damage = $damage->{"damage"}{$state->[0]}{$state->[1]};

					$total_state_damage->{"cut"} ||= 0.0;
					$total_state_damage->{"cut"} += $state_damage->{"cut"} * $rate;
					$total_state_damage->{"impact"} ||= 0.0;
					$total_state_damage->{"impact"} += $state_damage->{"impact"} * $rate;
					$total_state_damage->{"piercing"} ||= 0.0;
					$total_state_damage->{"piercing"} += $state_damage->{"piercing"} * $rate;
					$total_state_damage->{"fixed"} ||= 0.0;
					$total_state_damage->{"fixed"} += $state_damage->{"fixed"} * $rate;
					$total_state_damage->{"elements"} ||= {};
					for my $element (keys %{$state_damage->{"elements"}}) {
						$total_state_damage->{"elements"}{$element} ||= 0.0;
						$total_state_damage->{"elements"}{$element} +=
							$state_damage->{"elements"}{$element} *= $rate;
					}
					$total_state_damage->{"statuses"} ||= {};
					for my $status (keys %{$state_damage->{"statuses"}}) {
						$total_state_damage->{"statuses"}{$status} ||= 0.0;
						$total_state_damage->{"statuses"}{$status} +=
							$state_damage->{"statuses"}{$status} *= $rate;
					}

					for my $v (@{$state_damage->{"bounce_sharpness"}}) {
						my $rate_proportion = ($total_rate > 0.0 ? $rate / $total_rate : 1.0);
						push @{$total_state_damage->{"bounce_sharpness"}},
							[($v->[0] > 900 ? 999 : $v->[0] * $rate_proportion), $v->[1]];
					}
					my $new_bounce_sharpness = [];
					for my $v (sort { $b->[1] <=> $a->[1] } (@{$total_state_damage->{"bounce_sharpness"}})) {
						my $i = (scalar @{$new_bounce_sharpness}) - 1;
						if ($i >= 0 && $new_bounce_sharpness->[$i][1] == $v->[1]) {
							$new_bounce_sharpness->[$i][0] += $v->[0];
						} else {
							push @{$new_bounce_sharpness}, [$v->[0], $v->[1]];
						}
					}
					$total_state_damage->{"bounce_sharpness"} = $new_bounce_sharpness;
				}
			}
			push @damages, $damage;
		}
	}
}

for my $monster (@{$data->{"monsters"}}) {
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
					my $state_dps = {
						"raw" => 0.0,
						"element" => 0.0,
						"status" => 0.0,
						"fixed" => 0.0,
						"bounce_rate" => 0.0 } ;
					my $state_damage_normal = $damage->{"damage"}{$enraged_state}{"normal"};
					my $state_damage_weak = $damage->{"damage"}{$enraged_state}{"weak_spot"};

					my %hit_data_p = ("cut" => $hit_data->{"cut"}, "impact" => $hit_data->{"impact"});
					$hit_data_p{"piercing"} = $hit_data->{"impact"} * $constants->{"piercing_factor"};
					$hit_data_p{"piercing"} = $hit_data->{"cut"} if $hit_data_p{"piercing"} < $hit_data->{"cut"};

					my $state_damage_total = 0.0;
					for my $hit_type (keys %hit_data_p) {
						my $state_damage = $hit_data_p{$hit_type} >= $constants->{"weakness_threshold"} ?
							$state_damage_weak : $state_damage_normal;
						$state_damage_total += $state_damage->{$hit_type};
					}
					for my $hit_type (keys %hit_data_p) {
						my $state_damage = $hit_data_p{$hit_type} >= $constants->{"weakness_threshold"} ?
							$state_damage_weak : $state_damage_normal;
						$state_dps->{"raw"} += $state_damage->{$hit_type} * $hit_data_p{$hit_type} / 100.0;
						for my $v (@{$state_damage->{"bounce_sharpness"}}) {
							if ($v->[1] * $hit_data_p{$hit_type} < $constants->{"bounce_threshold"}) {
								$state_dps->{"bounce_rate"} +=
									$v->[0] * $state_damage->{$hit_type} / $state_damage_total;
							}
						}
					}

					for my $element (keys %{$state_damage_weak->{"elements"}}) {
						my $state_damage = $hit_data->{$element} >= $constants->{"element_weakness_threshold"} ?
							$state_damage_weak : $state_damage_normal;
						$state_dps->{"element"} +=
							$state_damage->{"elements"}{$element} * $hit_data->{$element} / 100.0;
					}

					$state_dps->{"fixed"} += $state_damage_weak->{"fixed"};

					for (1..1) {
						$state_dps->{"total"} =
							$state_dps->{"raw"} + $state_dps->{"element"} + $state_dps->{"status"} + $state_dps->{"fixed"};
						$state_dps->{"kill_freq"} = $state_dps->{"total"} / $constants->{"monster_hit_points"};
						if ($state_dps->{"kill_freq"} > 0) {
							for my $status (keys %{$state_damage_weak->{"statuses"}}) {
								my $status_attack = $state_damage_weak->{"statuses"}{$status};
								if (defined $hit_data->{$status}) {
									$status_attack *= $hit_data->{$status} / 100.0;
								}
								my $hits =
									get_status_hits($status_attack,
									                $monster->{"tolerances"}{$status},
									                1.0 / $state_dps->{"kill_freq"},
									                ($status ne "poison"));
								if ($hits > 0.0) {
									$state_dps->{"status"} += $hits * $monster->{"tolerances"}{$status}{"damage"};
									$state_dps->{"proc_rate"}{$status} = $hits;
								}
							}
						}
					}

					my $ratio = $constants->{"enraged_ratio"};
					$ratio = 1.0 - $ratio if ($enraged_state eq "!enraged");
					$ratio /= $nb_hit_data;
					for my $key (keys %{$state_dps}) {
						$dps->{$key} += $state_dps->{$key} * $ratio;
					}
				}
			}
			printf "%6.2f R:%6.2f E:%6.2f S:%6.2f F:%6.2f B:%4.2f K:%6.1f %s / %s / %s / %s / %s\n",
				($dps->{total}, $dps->{raw}, $dps->{element}, $dps->{status}, $dps->{fixed}, $dps->{bounce_rate},
				 (1.0 / $dps->{kill_freq}), $monster->{name}, $part->{name},
				 $damage->{weapon}{name}, $damage->{profile}{name},
				 ("$damage->{buff}{name}" . (defined $damage->{buff}{level} ? " [$damage->{buff}{level}]" : "")));
		}
	}
}
