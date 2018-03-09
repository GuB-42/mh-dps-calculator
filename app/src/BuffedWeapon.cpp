#include "BuffedWeapon.h"
#include "FoldedBuffs.h"

BuffedWeapon::BuffedWeapon(const Weapon &weapon, const FoldedBuffsData &buffs,
                           double sharpen_period, double sharpness_use,
                           double element_status_crit_adjustment) {

	affinity = weapon.affinity + buffs.normalBuffs[BUFF_AFFINITY_PLUS];

	attack = (weapon.attack * buffs.normalBuffs[BUFF_ATTACK_MULTIPLIER]) +
		buffs.normalBuffs[BUFF_ATTACK_PLUS];




/*
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

	if ($buff_data->{"minds_eye"} < 1.0) {
		$buffed_weapon->{"bounce_sharpness"} =
			get_sharp_bonus_array($weapon, $buff_data->{"sharpness_plus"}, $sharpness_use,
			                      $buff_data->{"max_sharpness_time"} / $constants->{"sharpen_period"},
			                      $constants->{"raw_sharpness_multipliers"});
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
			$power = compute_buffed_element($power,
			                                $buff_data->{"element_plus"}{$element},
			                                $buff_data->{"element_multiplier"}{$element},
			                                $constants->{"element_status_plus_caps"});
			$power += $buff_data->{"all_elements_plus"};
			$power *= $buff_data->{"all_elements_multiplier"};
			$buffed_weapon->{"elements"}{$element} = $power;
		}
	}
	for my $status (keys %{$weapon->{"statuses"}}) {
		my $power = $weapon->{"statuses"}{$status} / $multi_divider;
		if ($weapon->{"awakened"}) {
			$power *= $buff_data->{"awakening"};
		}
		if ($power > 0) {
			$power = compute_buffed_element($power,
			                                $buff_data->{"status_plus"}{$status},
			                                $buff_data->{"status_multiplier"}{$status},
			                                $constants->{"element_status_plus_caps"});
			$power += $buff_data->{"all_statuses_plus"};
			$power *= $buff_data->{"all_statuses_multiplier"};
			$buffed_weapon->{"statuses"}{$status} = $power;
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
*/
}

