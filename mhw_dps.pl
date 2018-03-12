#!/usr/bin/perl

use strict;
use warnings;

use open ':std', ':encoding(UTF-8)';

use FindBin;
use lib $FindBin::Bin;
use DataReader;

use Data::Dumper;
$Data::Dumper::Sortkeys = 1;

# time is in seconds

my $constants = {
	"raw_sharpness_multipliers" => [ 0.5, 0.75, 1.0, 1.05, 1.2, 1.32, 1.44 ],
	"element_sharpness_multipliers" => [ 0.25, 0.5, 0.75, 1.0, 1.0625, 1.125, 1.2 ],
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
	"element_status_plus_caps" => [
		[  1,  4 ], [ 12,  4 ], [ 15,  5 ], [ 21,  6 ], [ 24,  7 ], [ 27,  8 ],
		[ 30,  9 ], [ 33, 10 ], [ 36, 11 ], [ 39, 12 ], [ 42, 13 ], [ 48, 14 ],
		[ 54, 15 ] ],
	"element_status_crit_adjustment" => {
		"great_sword" => 0.8, # 0.25 => 0.2
		"sword_and_shield" => 1.4, # 0.25 => 0.35
		"dual_blades" => 1.4,
		"light_bowgun" => 1.4,
		"heavy_bowgun" => 1.4,
		"bow" => 1.4,
	},

	"enraged_ratio" => 0.4,
	"monster_defense_multiplier" => 0.5,
	"sharpness_use" => 15,
	"sharpen_period" => 300.0,
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
						$buff_combiners{$buff_key}->($acu->{$buff_key}{$elt_key}{$condition_key},
					                                 $other->{$buff_key}{$elt_key}{$condition_key});
				}
			}
		} else {
			for my $condition_key (keys %{$other->{$buff_key}}) {
				$acu->{$buff_key}{$condition_key} =
					$buff_combiners{$buff_key}->($acu->{$buff_key}{$condition_key},
				                                 $other->{$buff_key}{$condition_key});
			}
		}
	}
}

sub compute_buffed_element
{
	my ($base, $plus, $multiplier, $caps) = @_;

	my $val = $base || 0;
	$val += $plus if (defined $plus);
	$val *= $multiplier if (defined $multiplier);
	my $cap = 0;
	for my $v (@{$caps}) {
		if ($v->[0] <= $base) {
			$cap = $v->[1];
		} else {
			last;
		}
	}
	return $val > $base + $cap ? $base + $cap : $val;
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
	} elsif ($weapon->{"awakened"}) {
		if (defined $buff->{"awakening"}) {
			$conditions->{"raw_weapon"} = 1.0 - $buff->{"awakening"}{"always"};
		} else {
			$conditions->{"raw_weapon"} = 1.0;
		}
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
		"bounce_sharpness" => [],
		"minds_eye" => $buff_data->{"minds_eye"}
	};

	$buffed_weapon->{"affinity"} += $buff_data->{"affinity_plus"};
	$buffed_weapon->{"attack"} *= $buff_data->{"attack_multiplier"};
	$buffed_weapon->{"attack"} += $buff_data->{"attack_plus"};

	if ($constants->{"element_status_crit_adjustment"}{$weapon->{"type"}}) {
		my $mult = $constants->{"element_status_crit_adjustment"}{$weapon->{"type"}};
		$buffed_weapon->{"element_critical_hit_multiplier"} =
			(($buffed_weapon->{"element_critical_hit_multiplier"} - 1) * $mult) + 1;
		$buffed_weapon->{"status_critical_hit_multiplier"} =
			(($buffed_weapon->{"status_critical_hit_multiplier"} - 1) * $mult) + 1;
	}

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
		                $constants->{"raw_sharpness_multipliers"});
	$buffed_weapon->{"esharp"} =
		get_sharp_bonus($weapon, $buff_data->{"sharpness_plus"}, $sharpness_use,
		                $buff_data->{"max_sharpness_time"} / $constants->{"sharpen_period"},
		                $constants->{"element_sharpness_multipliers"});

	$buffed_weapon->{"bounce_sharpness"} =
		get_sharp_bonus_array($weapon, $buff_data->{"sharpness_plus"}, $sharpness_use,
		                      $buff_data->{"max_sharpness_time"} / $constants->{"sharpen_period"},
		                      $constants->{"raw_sharpness_multipliers"});

	my $multi_divider =
		scalar(%{$weapon->{"elements"}}) + scalar(%{$weapon->{"statuses"}});

	for my $element (keys %{$weapon->{"elements"}}) {
		my $power = $weapon->{"elements"}{$element};
		if ($power > 0) {
			$power = compute_buffed_element($power,
			                                $buff_data->{"element_plus"}{$element},
			                                $buff_data->{"element_multiplier"}{$element},
			                                $constants->{"element_status_plus_caps"});
			$power += $buff_data->{"all_elements_plus"};
			$power *= $buff_data->{"all_elements_multiplier"};
			if ($weapon->{"awakened"}) {
				$power *= $buff_data->{"awakening"};
			}
			$buffed_weapon->{"elements"}{$element} = $power / $multi_divider;
		}
	}
	for my $status (keys %{$weapon->{"statuses"}}) {
		my $power = $weapon->{"statuses"}{$status};
		if ($power > 0) {
			$power = compute_buffed_element($power,
			                                $buff_data->{"status_plus"}{$status},
			                                $buff_data->{"status_multiplier"}{$status},
			                                $constants->{"element_status_plus_caps"});
			$power += $buff_data->{"all_statuses_plus"};
			$power *= $buff_data->{"all_statuses_multiplier"};
			if ($weapon->{"awakened"}) {
				$power *= $buff_data->{"awakening"};
			}
			$buffed_weapon->{"statuses"}{$status} = $power / $multi_divider;
		}
	}
	for my $element (keys %{$weapon->{"phial_elements"}}) {
		my $power = $weapon->{"phial_elements"}{$element};
		$power = compute_buffed_element($power,
		                                $buff_data->{"element_plus"}{$element},
		                                $buff_data->{"element_multiplier"}{$element},
		                                $constants->{"element_status_plus_caps"});
		$power += $buff_data->{"all_elements_plus"};
		$power *= $buff_data->{"all_elements_multiplier"};
		$buffed_weapon->{"phial_elements"}{$element} = $power;
	}
	for my $status (keys %{$weapon->{"phial_statuses"}}) {
		my $power = $weapon->{"phial_statuses"}{$status};
		$power = compute_buffed_element($power,
		                                $buff_data->{"status_plus"}{$status},
		                                $buff_data->{"status_multiplier"}{$status},
		                                $constants->{"element_status_plus_caps"});
		$power += $buff_data->{"all_statuses_plus"};
		$power *= $buff_data->{"all_statuses_multiplier"};
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
		"minds_eye" => $buffed_weapon->{"minds_eye"}
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

	$damage->{"statuses"}{"exhaust"} = $exhaust if ($exhaust > 0.0);
	$damage->{"statuses"}{"stun"} = $stun if ($stun > 0.0);

	return $damage;
}

sub state_damage_sum
{
	my ($acu, $other, $rate, $total_rate) = @_;

	$acu->{"cut"} ||= 0.0;
	$acu->{"cut"} += $other->{"cut"} * $rate;
	$acu->{"impact"} ||= 0.0;
	$acu->{"impact"} += $other->{"impact"} * $rate;
	$acu->{"piercing"} ||= 0.0;
	$acu->{"piercing"} += $other->{"piercing"} * $rate;
	$acu->{"fixed"} ||= 0.0;
	$acu->{"fixed"} += $other->{"fixed"} * $rate;
	$acu->{"elements"} ||= {};
	for my $element (keys %{$other->{"elements"}}) {
		$acu->{"elements"}{$element} ||= 0.0;
		$acu->{"elements"}{$element} +=
			$other->{"elements"}{$element} *= $rate;
	}
	$acu->{"statuses"} ||= {};
	for my $status (keys %{$other->{"statuses"}}) {
		$acu->{"statuses"}{$status} ||= 0.0;
		$acu->{"statuses"}{$status} +=
			$other->{"statuses"}{$status} *= $rate;
	}

	for my $v (@{$other->{"bounce_sharpness"}}) {
		my $rate_proportion = ($total_rate > 0.0 ? $rate / $total_rate : 1.0);
		push @{$acu->{"bounce_sharpness"}},
			[($v->[0] > 900 ? 999 : $v->[0] * $rate_proportion), $v->[1]];
	}
	my $new_bounce_sharpness = [];
	for my $v (sort { $b->[1] <=> $a->[1] } (@{$acu->{"bounce_sharpness"}})) {
		my $i = (scalar @{$new_bounce_sharpness}) - 1;
		if ($i >= 0 && $new_bounce_sharpness->[$i][1] == $v->[1]) {
			$new_bounce_sharpness->[$i][0] += $v->[0];
		} else {
			push @{$new_bounce_sharpness}, [$v->[0], $v->[1]];
		}
	}
	$acu->{"bounce_sharpness"} = $new_bounce_sharpness;
	$acu->{"minds_eye"} ||= 0.0;
	$acu->{"minds_eye"} += $other->{"minds_eye"} * $rate / $total_rate;
}

sub get_damage_data
{
	my ($profile, $weapon, $buff_data) = @_;

	my $damage_data = {};

	for my $state ([ "not_enraged", "normal_spot" ],
	               [ "enraged", "normal_spot" ],
	               [ "not_enraged", "weak_spot" ],
	               [ "enraged", "weak_spot" ]) {
		my $total_rate = 0.0;
		for my $pattern (@{$profile->{"patterns"}}) {
			$total_rate += $pattern->{"rate"} / $pattern->{"period"};
		}
		for my $pattern (@{$profile->{"patterns"}}) {
			my $rate = $pattern->{"rate"} / $pattern->{"period"};
			my $buffed_weapon = compute_buffed_weapon($profile, $pattern, $weapon, $state, $buff_data);
			my $state_damage = compute_damage($profile, $pattern, $buffed_weapon);

			$damage_data->{$state->[0]} ||= {};
			$damage_data->{$state->[0]}{$state->[1]} ||= {};
			state_damage_sum($damage_data->{$state->[0]}{$state->[1]}, $state_damage, $rate, $total_rate);
		}
	}
	return $damage_data;
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

sub get_dps
{
	my ($monster, $hit_data, $state_damage_data, $monster_defense_multiplier) = @_;

	my $dps = {
		"raw" => 0.0,
		"element" => 0.0,
		"status" => 0.0,
		"fixed" => 0.0,
		"bounce_rate" => 0.0,
		"proc_rate" => {},
		"kill_freq" => 0.0
	};
	my $state_damage_normal = $state_damage_data->{"normal_spot"};
	my $state_damage_weak = $state_damage_data->{"weak_spot"};

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
		$dps->{"raw"} += $state_damage->{$hit_type} * $hit_data_p{$hit_type} / 100.0;
		for my $v (@{$state_damage->{"bounce_sharpness"}}) {
			if ($v->[1] * $hit_data_p{$hit_type} < $constants->{"bounce_threshold"}) {
				$dps->{"bounce_rate"} += (1.0 - $state_damage_data->{"minds_eye"}) *
					$v->[0] * $state_damage->{$hit_type} / $state_damage_total;
			}
		}
	}

	for my $element (keys %{$state_damage_weak->{"elements"}}) {
		my $state_damage = $hit_data->{$element} >= $constants->{"element_weakness_threshold"} ?
			$state_damage_weak : $state_damage_normal;
		$dps->{"element"} +=
			$state_damage->{"elements"}{$element} * $hit_data->{$element} / 100.0;
	}

	$dps->{"fixed"} += $state_damage_weak->{"fixed"};

	for (1..3) {
		$dps->{"total"} =
			$dps->{"raw"} + $dps->{"element"} + $dps->{"status"} + $dps->{"fixed"};

		my $real_total = $dps->{"total"} * $monster_defense_multiplier;
		$dps->{"status"} = 0;
		if ($monster->{"hit_points"} > 0 && $real_total > 0) {
			$dps->{"kill_freq"} = $real_total / $monster->{"hit_points"};
			for my $status (keys %{$state_damage_weak->{"statuses"}}) {
				my $status_attack = $state_damage_weak->{"statuses"}{$status};
				if (defined $hit_data->{$status}) {
					$status_attack *= $hit_data->{$status} / 100.0;
				}
				my $hits =
					get_status_hits($status_attack,
					                $monster->{"tolerances"}{$status},
					                1.0 / $dps->{"kill_freq"},
					                ($status ne "poison"));
				if ($hits > 0.0) {
					$dps->{"status"} += $hits * $monster->{"tolerances"}{$status}{"damage"};
					$dps->{"proc_rate"}{$status} = $hits;
				}
			}
		}
		last unless ($dps->{"status"} > 0);
	}

	return $dps;
}

sub clean_buff_data
{
	my ($buff_data, $weapon) = @_;

	my $res = {};

	for my $key (keys %{$buff_data}) {
		if ($key eq "element_plus" || $key eq "element_multiplier") {
			for my $element (keys %{$buff_data->{$key}}) {
				if ($weapon->{"elements"}{$element} || $weapon->{"phial_elements"}{$element}) {
					$res->{$key} ||= {};
					$res->{$key}{$element} = $buff_data->{$key}{$element};
				}
			}
		} elsif ($key eq "status_plus" || $key eq "status_multiplier") {
			for my $status (keys %{$buff_data->{$key}}) {
				if ($weapon->{"statuses"}{$status} || $weapon->{"phial_statuses"}{$status}) {
					$res->{$key} ||= {};
					$res->{$key}{$status} = $buff_data->{$key}{$status};
				}
			}
		} elsif ($key eq "all_elements_plus" || $key eq "all_elements_multiplier" ||
		         $key eq "element_critical_hit_multiplier") {
			if (%{$weapon->{"elements"}} || %{$weapon->{"phial_elements"}}) {
				$res->{$key} = $buff_data->{$key};
			}
		} elsif ($key eq "all_statuses_plus" || $key eq "all_statuses_multiplier" ||
		         $key eq "status_critical_hit_multiplier") {
			if (%{$weapon->{"statuses"}} || %{$weapon->{"phial_statuses"}}) {
				$res->{$key} = $buff_data->{$key};
			}
		} elsif ($key eq "awakening") {
			$res->{$key} = $buff_data->{$key} if ($weapon->{"awakened"});
		} elsif ($key eq "sharpness_plus") {
			$res->{$key} = $buff_data->{$key} if ($weapon->{"sharpness_plus"});
		} elsif ($key eq "artillery_multiplier") {
			# TODO: gunlance
			$res->{$key} = $buff_data->{$key} if ($weapon->{"phial"} eq "impact");
		} else {
			unless ((keys %{$buff_data->{$key}}) == 1 &&
			        $buff_data->{$key}{"raw_weapon"} &&
			        !$weapon->{"awakened"} &&
			        (%{$weapon->{"elements"}} || %{$weapon->{"statuses"}})) {
				$res->{$key} = $buff_data->{$key};
			}
		}
	}
	return $res;
}

sub append_item
{
	my ($input, $buff_group_map, $item, $use_slot) = @_;

	my $res = {
		"slots" => [],
		"buff_levels" => {},
		"used_items" => []
	};

	if (defined $item->{"slots"}) {
		my @slots = @{$item->{"slots"}};
		push @slots, @{$input->{"slots"}} if ($input->{"slots"});
		$res->{"slots"} = [ (sort { $b <=> $a } @slots) ];
	} else {
		if ($use_slot) {
			$res->{"slots"} = [ @{$input->{"slots"}} ] || [];
		} else {
			$res->{"slots"} = $input->{"slots"} || [];
		}
	}
	if ($use_slot && @{$res->{"slots"}}) {
		my $idx_min = 0;
		my $idx_min_val = 999;
		my $decoration_level = $item->{"decoration_level"} || 0;
		for my $i (0 .. (@{$res->{"slots"}} - 1)) {
			if ($res->{"slots"}[$i] < $idx_min_val &&
			    $res->{"slots"}[$i] >= $decoration_level) {
				$idx_min = $i;
				$idx_min_val = $res->{"slots"}[$i];
			}
		}
		splice @{$res->{"slots"}}, $idx_min, 1;
	}
	if (defined $item->{"buff_refs"}) {
		my %levels = %{$input->{"buff_levels"}};
		for my $buff_ref (@{$item->{"buff_refs"}}) {
			my $group_id = $buff_ref->{"id"};
			my $level = $buff_ref->{"level"};
			$level += $levels{$group_id} if ($levels{$group_id});
			if ($level >= @{$buff_group_map->{$group_id}}) {
				$level = @{$buff_group_map->{$group_id}} - 1;
			}
			$levels{$group_id} = $level;
		}
		$res->{"buff_levels"} = { %levels };
	} else {
		$res->{"buff_levels"} = $input->{"buff_levels"};
	}
	$res->{"used_items"} = [ @{$input->{"used_items"}}, $item ];
	return $res;
}

sub fill_slots
{
	my ($output_list, $input, $buff_group_map, @items) = @_;

	if ($input->{"slots"} && @{$input->{"slots"}}) {
		my $max_slot = 0;
		for my $slot (@{$input->{"slots"}}) {
			$max_slot = $slot if ($slot > $max_slot);
		}
		my @useful_items = ();
		for my $item (@items) {
			if ($item->{"buff_refs"} &&
			    $item->{"decoration_level"} &&
			    $item->{"decoration_level"} <= $max_slot) {
				for my $buff_ref (@{$item->{"buff_refs"}}) {
					my $group_id = $buff_ref->{"id"};
					if ($input->{"buff_levels"} && $input->{"buff_levels"}{$group_id}) {
						if ($input->{"buff_levels"}{$group_id} < @{$buff_group_map->{$group_id}} - 1) {
							push @useful_items, $item;
							last;
						}
					} else {
						push @useful_items, $item;
						last;
					}
				}
			}
		}
		while (@useful_items) {
			my $new_input = append_item($input, $buff_group_map, $useful_items[0], 1);
			fill_slots($output_list, $new_input, $buff_group_map, @useful_items);
			shift @useful_items;
		}
	}
	push @{$output_list}, $input;
}

sub get_useful_items
{
	my ($items, $buff_group_map, $weapon) = @_;

	my @res = ();

	my %useful_buffs = ();
	for my $item (@{$items}) {
		next unless $item->{"buff_refs"};
		for my $buff_ref (@{$item->{"buff_refs"}}) {
			my $group_id = $buff_ref->{"id"};
			if (!defined $useful_buffs{$group_id}) {
				$useful_buffs{$group_id} = 0;
				for my $buff (@{$buff_group_map->{$group_id}}) {
					my $d = clean_buff_data($buff->{"data"}, $weapon);
					if (%{$d}) {
						$useful_buffs{$group_id} = 1;
						last;
					}
				}
			}
			if ($useful_buffs{$group_id}) {
				push @res, $item;
				last;
			}
		}
	}
	return @res;
}

sub dps_sum
{
	my ($acu, $other, $rate) = @_;
	$acu->{"total"} ||= 0.0;
	$acu->{"total"} += $other->{"total"} * $rate;
	$acu->{"raw"} ||= 0.0;
	$acu->{"raw"} += $other->{"raw"} * $rate;
	$acu->{"element"} ||= 0.0;
	$acu->{"element"} += $other->{"element"} * $rate;
	$acu->{"status"} ||= 0.0;
	$acu->{"status"} += $other->{"status"} * $rate;
	$acu->{"fixed"} ||= 0.0;
	$acu->{"fixed"} += $other->{"fixed"} * $rate;
	$acu->{"bounce_rate"} ||= 0.0;
	$acu->{"bounce_rate"} += $other->{"bounce_rate"} * $rate;
	for my $proc (keys %{$acu->{"proc_rate"}}) {
		$acu->{"proc_rate"}{$proc} ||= 0.0;
		$acu->{"proc_rate"}{$proc} += $other->{"proc_rate"} * $rate;
	}
	$acu->{"kill_freq"} ||= 0.0;
	$acu->{"kill_freq"} += $other->{"kill_freq"} * $rate;
}

sub write_damage_debug_csv
{
	open F, '>', "debug.csv" || die "$!";
	for my $damage (@_) {
		my @items = ();
		for (my $i = 1; $i < @{$damage->{used_items}}; ++$i) {
			push @items, $damage->{used_items}[$i]{"name"};
		}
		my $item_names = (join " / ", sort @items);
		for my $state ([ "not_enraged", "normal_spot" ],
		               [ "enraged", "normal_spot" ],
		               [ "not_enraged", "weak_spot" ],
		               [ "enraged", "weak_spot" ]) {
			my $dd = $damage->{"damage_data"}{$state->[0]}{$state->[1]};
			printf F "%s,%s,%s,%s,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g\n",
				$damage->{"profile"}{"name"},
				$damage->{"weapon"}{"name"},
				$item_names,
				"$state->[0]/$state->[1]",
				$dd->{"cut"}, $dd->{"impact"}, $dd->{"piercing"}, $dd->{"fixed"},
				($dd->{"elements"}{"fire"} || 0.0),
				($dd->{"elements"}{"thunder"} || 0.0),
				($dd->{"elements"}{"ice"} || 0.0),
				($dd->{"elements"}{"dragon"} || 0.0),
				($dd->{"elements"}{"water"} || 0.0),
				($dd->{"statuses"}{"poison"} || 0.0),
				($dd->{"statuses"}{"paralysis"} || 0.0),
				($dd->{"statuses"}{"sleep"} || 0.0),
				($dd->{"statuses"}{"stun"} || 0.0),
				($dd->{"statuses"}{"blast"} || 0.0),
				($dd->{"statuses"}{"exhaust"} || 0.0),
				($dd->{"statuses"}{"mount"} || 0.0),
				($dd->{"minds_eye"} || 0.0),
				($dd->{"bounce_sharpness"}[0][0]),
				($dd->{"bounce_sharpness"}[0][1]);
		}
	}
	close F;
}

my $data = {};
parse_data_files($data, @ARGV);

my @damages = ();
for my $profile (@{$data->{"profiles"}}) {
	for my $weapon (@{$data->{"weapons"}}) {
		next if ($weapon->{"type"} ne $profile->{"type"});
		next unless ($weapon->{"final"});

		my $start_build = {
			"slots" => [ 3 ],
			"buff_levels" => {},
			"used_items" => []
		};
		$start_build = append_item($start_build, $data->{"buff_group_map"}, $weapon);
		$start_build = append_item($start_build, $data->{"buff_group_map"}, $data->{"item_map"}{"powercharm"});
		$start_build = append_item($start_build, $data->{"buff_group_map"}, $data->{"item_map"}{"powertalon"});
#		$start_build->{"slots"} = [ 1 ];
		my @gen_builds = ();
		fill_slots(\@gen_builds, $start_build, $data->{"buff_group_map"},
		           get_useful_items($data->{"items"}, $data->{"buff_group_map"}, $weapon));

		for my $build (@gen_builds) {
			my $buff_data = {};
			for my $group_id (keys %{$build->{"buff_levels"}}) {
				my $level = $build->{"buff_levels"}{$group_id};
				all_buff_combine($buff_data, $data->{"buff_group_map"}{$group_id}[$level]{"data"});
			}
			my $damage = {
				"weapon" => $weapon,
				"profile" => $profile,
				"used_items" => $build->{"used_items"},
				"slots" => $build->{"slots"},
				"buff_levels" => $build->{"buff_levels"},
				"damage_data" => get_damage_data($profile, $weapon, $buff_data)
			};
			push @damages, $damage;
		}
	}
}

my @results;

for my $damage (@damages) {
	for my $target (@{$data->{"targets"}}) {
		my $total_weight = 0.0;
		my $total_dps = {};

		for my $sub_target (@{$target->{"sub_targets"}}) {
			my $weight = $sub_target->{"weight"};
			my $monster_name = defined $sub_target->{"monster_name"} ?
				$sub_target->{"monster_name"} : $target->{"monster_name"};
			my $part_name = defined $sub_target->{"part_name"} ?
				$sub_target->{"part_name"} : $target->{"part_name"};
			my $state = defined $sub_target->{"state"} ?
				$sub_target->{"state"} : $target->{"state"};
			my $enraged_ratio = defined $sub_target->{"enraged_ratio"} ?
				$sub_target->{"enraged_ratio"} : defined $target->{"enraged_ratio"} ?
				$target->{"enraged_ratio"} : $constants->{"enraged_ratio"};
			my $monster_defense_multiplier = defined $sub_target->{"monster_defense_multiplier"} ?
				$sub_target->{"monster_defense_multiplier"} : defined $target->{"monster_defense_multiplier"} ?
				$target->{"enraged_ratio"} : $constants->{"enraged_ratio"};

			my $sub_target_dps = {};
			my $sub_target_weight = 0.0;
			for my $monster (@{$data->{"monsters"}}) {
				my $monster_dps = {};
				my $monster_weight = 0.0;
				next if (defined $monster_name && $monster_name ne $monster->{"name"});
				for my $part (@{$monster->{"parts"}}) {
					my $part_dps = {};
					my $part_weight = 0.0;
					next if (defined $part_name && $part_name ne $part->{"name"});
					for my $hit_data (@{$part->{"hit_data"}}) {
						next if (defined $state && defined $hit_data->{"state"} &&
						         $state ne $hit_data->{"state"});
						my $hit_data_dps;
						$hit_data_dps = get_dps($monster, $hit_data, $damage->{"damage_data"}{"enraged"},
						                        $monster_defense_multiplier);
						dps_sum($part_dps, $hit_data_dps, $enraged_ratio);
						$hit_data_dps = get_dps($monster, $hit_data, $damage->{"damage_data"}{"not_enraged"},
						                        $monster_defense_multiplier);
						dps_sum($part_dps, $hit_data_dps, 1.0 - $enraged_ratio);
						$part_weight += 1.0;
					}
					if ($part_weight > 0.0) {
						dps_sum($monster_dps, $part_dps, 1.0 / $part_weight);
						$monster_weight += 1.0;
					}
				}
				if ($monster_weight > 0.0) {
					dps_sum($sub_target_dps, $monster_dps, 1.0 / $monster_weight);
					$sub_target_weight += 1.0;
				}
			}
			if ($sub_target_weight > 0.0) {
				dps_sum($total_dps, $sub_target_dps, $weight / $sub_target_weight);
				$total_weight += $weight;
			}
		}

		my $dps = {};
		if ($total_weight != 0.0) {
			dps_sum($dps, $total_dps, 1.0 / $total_weight);
		}

		my $result = {
			"target" => $target,
			"profile" => $damage->{"profile"},
			"weapon" => $damage->{"weapon"},
			"used_items" => $damage->{"used_items"},
			"slots" => $damage->{"slots"},
			"buff_levels" => $damage->{"buff_levels"},
			"damage_data" => $damage->{"damage_data"},
			"dps" => $dps
		};
		push @results, $result;
		printf "%6.2f R:%6.2f E:%6.2f S:%6.2f F:%6.2f B:%4.2f K:%6.1f %s / %s / %s\n",
			($result->{dps}{total},
			 $result->{dps}{raw}, $result->{dps}{element}, $result->{dps}{status}, $result->{dps}{fixed},
			 $result->{dps}{bounce_rate}, $result->{dps}{kill_freq} == 0 ? 0 : (1.0 / $result->{dps}{kill_freq}),
			 $result->{target}{name}, $result->{profile}{name},
			 (join " / ", (map { $_->{"name"} } @{$result->{used_items}})));
	}
}

exit;

my %res_by_profile = ();
for my $result (@results) {
	$res_by_profile{$result->{"profile"}{"name"}} ||= [];
	push @{$res_by_profile{$result->{"profile"}{"name"}}}, $result;
}
for my $pname (sort keys %res_by_profile) {
	my @best_of_weapon = ();
	my %res_by_weapon = ();
	for my $result (@{$res_by_profile{$pname}}) {
		$res_by_weapon{$result->{"weapon"}{"name"}} ||= [];
		push @{$res_by_weapon{$result->{"weapon"}{"name"}}}, $result;
	}
	for my $wname (sort keys %res_by_weapon) {
		@{$res_by_weapon{$wname}} = sort { $b->{dps}{total} <=> $a->{dps}{total} } @{$res_by_weapon{$wname}};
		push @best_of_weapon, $res_by_weapon{$wname}[0];
	}
	@best_of_weapon = sort { $b->{dps}{total} <=> $a->{dps}{total} } @best_of_weapon;

	open F, '>', "out_csv/$pname.csv" || die "$!";
	print F "Total DPS,Raw DPS,Element DPS,Status DPS,Fixed DPS,Bounce,Weapon,Arme,Buff1,Buff2,Buff3,Buff4\n";
	for my $result (@best_of_weapon) {
		printf F "%g,%g,%g,%g,%g,%s,%s,%s,%s,%s,%s\n",
			($result->{dps}{total},
			 $result->{dps}{raw}, $result->{dps}{element}, $result->{dps}{status}, $result->{dps}{fixed},
			 $result->{dps}{bounce_rate}, $result->{weapon}{name}, $result->{weapon}{name_fr},
			 $result->{used_items}[3]{name}, $result->{used_items}[4]{name},
			 $result->{used_items}[5]{name}, $result->{used_items}[6]{name});
	}
	close F
}
