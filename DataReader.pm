package DataReader;

use strict;
use warnings;

BEGIN {
	require Exporter;
	our $VERSION = 1.00;
	our @ISA = qw(Exporter);
	our @EXPORT = qw(parse_data_files);
	our @EXPORT_OK = qw();
}

use XML::Parser;
use Data::Dumper;

my $output_data;

my @xml_stack;
my @xml_string_stack;
my @xml_stack_attr;

my $cur_weapon = {};
my $cur_part = {};
my $cur_hit_data = {};
my $cur_tolerance = {};
my $cur_monster = {};
my $cur_profile = {};
my $cur_pattern = {};
my $cur_buff_group = [];
my $cur_buff = {};
my $cur_buff_condition = "always";
my $cur_buff_level = 1;
my $cur_item = {};
my $cur_target = {};
my $cur_sub_target = {};

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

my %buff_conditions = (
	"always" => 0,
	"weak_spot" => 1,
	"enraged" => 2,
	"raw_weapon" => 3,
	"draw_attack" => 4,
	"airborne" => 5,
	"red_life" => 6,
	"full_life" => 7,
	"death_1" => 8,
	"death_2" => 9,
	"full_stamina" => 10,
	"sliding" => 11,
	"low_life" => 12,
	"received_damage" => 13,
	"might_seed_use" => 14,
	"demon_powder_use" => 15,
	"might_pill_use" => 16
);

# special cases for elements and statuses
my %buff_codes = (
	"attack_plus" => 0,
	"attack_multiplier" => 1,
	"affinity_plus" => 2,
	"all_elements_plus" => 3,
	"all_elements_multiplier" => 4,
	"all_statuses_plus" => 5,
	"all_statuses_multiplier" => 6,
	"awakening" => 7,
	"sharpness_plus" => 8,
	"max_sharpness_time" => 9,
	"sharpness_use_multiplier" => 10,
	"sharpness_use_critical_multiplier" => 11,
	"draw_attack_stun" => 12,
	"draw_attack_exhaust" => 13,
	"stun_multiplier" => 14,
	"exhaust_multiplier" => 15,
	"artillery_multiplier" => 16,
	"raw_critical_hit_multiplier" => 17,
	"element_critical_hit_multiplier" => 18,
	"status_critical_hit_multiplier" => 19,
	"minds_eye" => 20
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
			"sharpness_plus" => 0,
			"slots" => [],
			"rare" => 0
		};
	} elsif ($xml_stack[0] eq "monster") {
		$cur_monster = {
			"name" => "no_name",
			"hit_points" => 0,
			"parts" => [],
			"tolerances" => {}
		};
		for my $tol ("poison", "sleep", "paralysis", "stun", "exhaust", "blast", "mount", "jump") {
			$cur_monster->{"tolerances"}{$tol} = {
				"initial" => 0,
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
			"period" => 1.0,
			"cut" => 0.0,
			"impact" => 0.0,
			"piercing" => 0.0,
			"element" => 0.0,
			"stun" => 0.0,
			"exhaust" => 0.0,
			"sharpness_multiplier" => 1.0,
			"phial_impact_attack" => 0.0,
			"phial_impact_stun" => 0.0,
			"phial_impact_exhaust" => 0.0,
			"phial_element_attack" => 0.0,
			"phial_ratio" => 0.0,
			"draw_attack" => 0.0
		}
	} elsif ($xml_stack[0] eq "buff_group") {
		$cur_buff_group = [ { "name" => "none", "data" => {} } ];
	} elsif ($xml_stack[0] eq "buff") {
		$cur_buff = {
			"name" => "no_name",
			"data" => {}
		};
		$cur_buff_condition = "always";
		$cur_buff_level = 1;
	} elsif ($xml_stack[1] eq "buff" && defined $buff_conditions{$xml_stack[0]}) {
		$cur_buff_condition = $xml_stack[0];
	} elsif ($xml_stack[0] eq "item") {
		$cur_item = {
			"name" => "no_name",
			"buff_refs" => []
		};
	} elsif ($xml_stack[0] eq "target") {
		$cur_target = {
			"name" => "no_name",
			"sub_targets" => []
		};
	} elsif ($xml_stack[1] eq "target" && $xml_stack[0] eq "sub_target") {
		$cur_sub_target = {
			"weight" => 1.0
		};
	}
}

sub process_val
{
	my $val = $xml_string_stack[0];
	$val =~ s/^\s+|\s+$//g;

	if ($xml_stack[0] eq "weapon") {
		push @{$output_data->{"weapons"}}, $cur_weapon;
	} elsif ($xml_stack[1] eq "weapon") {
		if ($xml_stack[0] eq "element" ||
		    $xml_stack[0] eq "sharpness" ||
		    $xml_stack[0] eq "phial" ||
		    $xml_stack[0] eq "slots") {
		    ; #do nothing
		} elsif ($xml_stack[0] eq "awakened") {
			if (lc($val) eq "true" || $val >= 1) {
				$cur_weapon->{"awakened"} = 1;
			}
		} else {
			$cur_weapon->{$xml_stack[0]} = $val;
		}
	} elsif ($xml_stack[2] eq "weapon" && $xml_stack[1] eq "element") {
		if (defined $element_codes{$xml_stack[0]}) {
			$cur_weapon->{"elements"}{$xml_stack[0]} = $val;
		} elsif (defined $status_codes{$xml_stack[0]}) {
			$cur_weapon->{"statuses"}{$xml_stack[0]} = $val;
		}
	} elsif ($xml_stack[2] eq "weapon" && $xml_stack[1] eq "sharpness") {
		if (defined $sharpness_levels{$xml_stack[0]}) {
			$cur_weapon->{"sharpness"}[$sharpness_levels{$xml_stack[0]}] = $val;
		} elsif ($xml_stack[0] eq "plus") {
			$cur_weapon->{"sharpness_plus"} = $val;
		}
	} elsif ($xml_stack[2] eq "weapon" && $xml_stack[1] eq "phial") {
		if ($xml_stack[0] eq "impact" ||
		    $xml_stack[0] eq "element" ||
		    $xml_stack[0] eq "power") {
			$cur_weapon->{"phial"} = $xml_stack[0];
		} elsif (defined $element_codes{$xml_stack[0]}) {
			$cur_weapon->{"phial_elements"}{$xml_stack[0]} = $val;
		} elsif (defined $status_codes{$xml_stack[0]}) {
			$cur_weapon->{"phial_statuses"}{$xml_stack[0]} = $val;
		}
	} elsif ($xml_stack[2] eq "weapon" && $xml_stack[1] eq "slots" && $xml_stack[0] eq "slot") {
		push @{$cur_weapon->{"slots"}}, $val;
	} elsif ($xml_stack[0] eq "monster") {
		push @{$output_data->{"monsters"}}, $cur_monster;
	} elsif ($xml_stack[1] eq "monster") {
		if ($xml_stack[0] eq "part") {
			my $has_states = 0;
			for my $hit_data (@{$cur_part->{"hit_data"}}) {
				if (defined $hit_data->{"state"}) {
					$has_states = 1;
					last;
				}
			}
			if ($has_states) {
				for my $hit_data (@{$cur_part->{"hit_data"}}) {
					$hit_data->{"state"} = "" unless (defined $hit_data->{"state"});
				}
			}
			push @{$cur_monster->{"parts"}}, $cur_part;
		} elsif ($xml_stack[0] eq "tolerance") {
			my $tol_type = $cur_tolerance->{"type"};
			delete $cur_tolerance->{"type"};
			$cur_monster->{"tolerances"}{$tol_type} = $cur_tolerance;
		} else {
			$cur_monster->{$xml_stack[0]} = $val;
		}
	} elsif ($xml_stack[1] eq "part") {
		if ($xml_stack[0] eq "name") {
			$cur_part->{"name"} = $val;
		} elsif ($xml_stack[0] eq "hit_data") {
			push @{$cur_part->{"hit_data"}}, $cur_hit_data;
		}
	} elsif ($xml_stack[1] eq "hit_data") {
		$cur_hit_data->{$xml_stack[0]} = $val;
	} elsif ($xml_stack[1] eq "tolerance") {
		$cur_tolerance->{$xml_stack[0]} = $val;
	} elsif ($xml_stack[0] eq "weapon_profile") {
		push @{$output_data->{"profiles"}}, $cur_profile;
	} elsif ($xml_stack[1] eq "weapon_profile") {
		if ($xml_stack[0] eq "pattern") {
			if (!defined $cur_pattern->{"status"}) {
				$cur_pattern->{"status"} = $cur_pattern->{"element"};
			}
			push @{$cur_profile->{"patterns"}}, $cur_pattern;
		} else {
			$cur_profile->{$xml_stack[0]} = $val;
		}
	} elsif ($xml_stack[1] eq "pattern") {
		$cur_pattern->{$xml_stack[0]} = $val;
	} elsif ($xml_stack[0] eq "buff_group") {
		push @{$output_data->{"buff_groups"}}, $cur_buff_group;
		if (defined $xml_stack_attr[0]->{"id"}) {
			$output_data->{"buff_group_map"}{$xml_stack_attr[0]->{"id"}} = $cur_buff_group;
		}
	} elsif ($xml_stack[0] eq "buff") {
		push @{$output_data->{"buffs"}}, $cur_buff;
		if (@{$cur_buff_group} <= $cur_buff_level) {
			while (@{$cur_buff_group} < $cur_buff_level) {
				push @{$cur_buff_group}, $cur_buff_group->[@{$cur_buff_group} - 1];
			}
			push @{$cur_buff_group}, $cur_buff;
		} else {
			$cur_buff_group->[$cur_buff_level] = $cur_buff;
		}
	} elsif (($xml_stack[2] eq "buff" && defined $buff_conditions{$xml_stack[1]}) ||
	         ($xml_stack[1] eq "buff")) {
		if ($xml_stack[0] eq "level") {
			$cur_buff_level = $val;
		} elsif ($xml_stack[0] eq "name" ||
		    $xml_stack[0] eq "decoration_level" ||
		    $xml_stack[0] eq "decoration_rarity") {
			$cur_buff->{$xml_stack[0]} = $val;
		} elsif (defined $buff_conditions{$xml_stack[0]}) {
			$cur_buff_condition = "always";
		} elsif (defined $buff_codes{$xml_stack[0]}) {
			$cur_buff->{"data"}{$xml_stack[0]} ||= {};
			$cur_buff->{"data"}{$xml_stack[0]}{$cur_buff_condition} = $val;
		}
	} elsif (($xml_stack[3] eq "buff" && defined $buff_conditions{$xml_stack[2]} && $xml_stack[1] eq "element_plus") ||
	         ($xml_stack[2] eq "buff" && $xml_stack[1] eq "element_plus")) {
		if (defined $element_codes{$xml_stack[0]}) {
			$cur_buff->{"data"}{"element_plus"} ||= {};
			$cur_buff->{"data"}{"element_plus"}{$xml_stack[0]} ||= {};
			$cur_buff->{"data"}{"element_plus"}{$xml_stack[0]}{$cur_buff_condition} = $val;
		} elsif (defined $status_codes{$xml_stack[0]}) {
			$cur_buff->{"data"}{"status_plus"} ||= {};
			$cur_buff->{"data"}{"status_plus"}{$xml_stack[0]} ||= {};
			$cur_buff->{"data"}{"status_plus"}{$xml_stack[0]}{$cur_buff_condition} = $val;
		}
	} elsif (($xml_stack[3] eq "buff" && defined $buff_conditions{$xml_stack[2]} && $xml_stack[1] eq "element_multiplier") ||
	         ($xml_stack[2] eq "buff" && $xml_stack[1] eq "element_multiplier")) {
		if (defined $element_codes{$xml_stack[0]}) {
			$cur_buff->{"data"}{"element_multiplier"} ||= {};
			$cur_buff->{"data"}{"element_multiplier"}{$xml_stack[0]} ||= {};
			$cur_buff->{"data"}{"element_multiplier"}{$xml_stack[0]}{$cur_buff_condition} = $val;
		} elsif (defined $status_codes{$xml_stack[0]}) {
			$cur_buff->{"data"}{"status_multiplier"} ||= {};
			$cur_buff->{"data"}{"status_multiplier"}{$xml_stack[0]} ||= {};
			$cur_buff->{"data"}{"status_multiplier"}{$xml_stack[0]}{$cur_buff_condition} = $val;
		}
	} elsif ($xml_stack[0] eq "item") {
		push @{$output_data->{"items"}}, $cur_item;
		if (defined $xml_stack_attr[0]->{"id"}) {
			$output_data->{"item_map"}{$xml_stack_attr[0]->{"id"}} = $cur_item;
		}
	} elsif ($xml_stack[1] eq "item") {
		if ($xml_stack[0] ne "buff_refs") {
			$cur_item->{$xml_stack[0]} = $val;
		}
	} elsif ($xml_stack[2] eq "item" && $xml_stack[1] eq "buff_refs" && $xml_stack[0] eq "buff_ref") {
		my $ref = { "id" => $xml_stack_attr[0]->{"id"} };
		$ref->{"level"} = defined $xml_stack_attr[0]->{"level"} ? $xml_stack_attr[0]->{"level"} : 1;
		push @{$cur_item->{"buff_refs"}}, $ref;
	} elsif ($xml_stack[0] eq "target") {
		unless (@{$cur_target->{"sub_targets"}}) {
			push @{$cur_target->{"sub_targets"}}, { "weight" => 1.0 };
		}
		push @{$output_data->{"targets"}}, $cur_target;
	} elsif ($xml_stack[1] eq "target") {
		if ($xml_stack[0] eq "sub_target") {
			push @{$cur_target->{"sub_targets"}}, $cur_sub_target;
		} else {
			$cur_target->{$xml_stack[0]} = $val;
		}
	} elsif ($xml_stack[2] eq "target" && $xml_stack[1] eq "sub_target") {
		$cur_sub_target->{$xml_stack[0]} = $val;
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
sub parse_data_files
{
	$output_data = shift @_;
	$output_data->{"monsters"} ||= [];
	$output_data->{"buffs"} ||= [];
	$output_data->{"buff_groups"} ||= [];
	$output_data->{"buff_group_map"} ||= {};
	$output_data->{"weapons"} ||= [];
	$output_data->{"profiles"} ||= [];
	$output_data->{"items"} ||= [];
	$output_data->{"item_map"} ||= {};
	$output_data->{"targets"} ||= [];
	for my $file (@_) {
		my $p = new XML::Parser(Handlers => {
			Start => \&handle_start,
			End => \&handle_end,
			Char => \&handle_char });
		@xml_string_stack = ();
		@xml_stack = ("null", "null", "null");
		$p->parsefile($file);
	}
}
