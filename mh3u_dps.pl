#!/usr/bin/perl -w

# Mon_Name_0|Mon_Hitzone_Name_0|Mon_ID|Mon_Hitzone_ID|Cutting|Impact|Bullet|Fire|Water|Ice|Thunder|Dragon|Stun|Exhaust
# select mn.mon_name_0, mz.mon_hitzone_name_0, mw.* from db_mon_weakness mw natural join id_mon_name mn natural join id_mon_hitzone mz;

# Wpn_Name_0|Wpn_ID|Wpn_Type_ID|Wpn_Sub_ID|Wpn_Rare|Wpn_Atk|Wpn_SpAtk1_ID|Wpn_SpAtk1_Num|Wpn_SpAtk2_ID|Wpn_SpAtk2_Num|Wpn_SpAtk_H|Wpn_Crit|Wpn_Def|Wpn_Slot|Wpn_ProPx|Wpn_LvUpPx|Wpn_Sharp0_1|Wpn_Sharp0_2|Wpn_Sharp0_3|Wpn_Sharp0_4|Wpn_Sharp0_5|Wpn_Sharp0_6|Wpn_Sharp0_7|Wpn_Sharp0|Wpn_Sharp1_1|Wpn_Sharp1_2|Wpn_Sharp1_3|Wpn_Sharp1_4|Wpn_Sharp1_5|Wpn_Sharp1_6|Wpn_Sharp1_7|Wpn_Sharp1
# select wn.wpn_name_0, w.* from db_wpn w natural join id_wpn_name wn where w.wpn_type_id = 4;

# cat vals | perl -e 'while (<>) { print "$_ : "; for $m (10..60) { if (int($_ / $m) == int($_ * 10 / $m) / 10) { print "$m "; $nbrs{$m}++ } } print "\n"; }; for (keys %nbrs) {print "$_ : $nbrs{$_}\n"}'

use strict;
use Data::Dumper;

my @wpns;

# my @wty_cut =    ( 0.1,  0.1,  0.1,   0.115, 0,   0,    0.1, 0.1,  0.1,  0,     0,     0 );
# my @wty_impact = ( 0,    0,    0,     0,     0.1, 0.1,  0,   0,    0,    0,     0,     0 );
# my @wty_elt =    ( 0.1,  0.1,  0.1,   0.645, 0.1, 0.1,  0.1, 0.1,  0.1,  0,     0,     0 );

my %dcpy_classAttSpeeds = (
		# 						X Combo, 				A Combo, 				XA Jump Slash
		"Sword_and_Shield" 	=>[[2.883, 4, 0.44, 15], 	[2.675, 3, 0.59, 0], 	[1.735, 1, 0.16, 0]],
		# 						X Combo,				A Combo,				Demon Dance
		"Dual_Sword" 		=>[[3.904, 7, 0.97, 0],		[2.405, 5, 0.51, 0],	[3.27, 7, 0.97, 0]],		# ORIGINAL
		# "Dual_Sword" 		=>[[3.104, 7, 0.97, 0],		[1.905, 5, 0.51, 0],	[2.72, 7, 0.97, 0]],		# MODIFIED
		# 						Step-slash sweep, 		Thrust Slash Sweep, 	Spirit Charge
		"Long_Sword" 		=>[[4.601, 3, 0.81, 0],		[3.416, 3, 0.7, 0]],
		# 						Charge, 				XA-X Swing, 			A-X Swing
		"Great_Sword" 		=>[[5.928, 2, 1.28, 27],	[4.501, 2, 0.54, 27],	[3.261, 2, 0.64, 27]],
		# 						X Three Pound, 			R Superpound, 			R Spin-Charge
		"Hammer" 			=>[[4.456, 3, 1.37, 57], 	[5.643, 2, 0.96, 75], 	[5.348, 7, 1.6, 48]],
		# 						Left-Right Combo,		Back pound front pound,	Recital
		"Hunting_Horn" 		=>[[2.973, 2, 0.6, 40],		[4.435, 3, 1.05, 45],	[4.155, 1, 0.35, 22]],
		# 						Triple Stab, 			Charge Finisher, 		Sweep
		"Lance" 			=>[[3.290, 3, 0.86, 0], 	[3.786, 1, 0.4, 0], 	[2.508, 1, 0.2, 0]],
		# 						Infinite Guard Stab,	Running combo,
		"Gunlance" 			=>[[0.850, 1, 0.18, 0],		[6.213, 4, 1.2, 0]],							# ORIGINAL
		# "Gunlance" 			=>[[1.850, 1, 0.18, 0],		[6.213, 4, 1.2, 0]],		 						# MODIFIED
		# 						Axe Combo				Sword Combo,			Discharge
		"Switch_Axe" 		=>[[3.768, 3, 1.17, 0],		[4.220, 4, 1.16, 0], 	[5.86, 8, 1.8, 0]]
);

# HBG no limiter = 1.7 mult
my @wclasses = (
	{ "name" => "GS",  "divider" => 4.8,  "cut" => 0.1,   "impact" => 0,     "piercing" => 0,   "shell" => 0,   "elt" => 0.1,   "ko" => 0,  },
	{ "name" => "LS",  "divider" => 3.3,  "cut" => 0.188, "impact" => 0,     "piercing" => 0,   "shell" => 0,   "elt" => 0.748, "ko" => 0,  },
	{ "name" => "SnS", "divider" => 1.4,  "cut" => 0.149, "impact" => 0,     "piercing" => 0,   "shell" => 0,   "elt" => 1.097, "ko" => 2.1 },
	{ "name" => "DS",  "divider" => 1.4,  "cut" => 0.339, "impact" => 0,     "piercing" => 0,   "shell" => 0,   "elt" => 1.721, "ko" => 0,  },
	{ "name" => "H",   "divider" => 5.2,  "cut" => 0,     "impact" => 0.254, "piercing" => 0,   "shell" => 0,   "elt" => 0.777, "ko" => 15, },
	{ "name" => "HH",  "divider" => 4.6,  "cut" => 0,     "impact" => 0.1,   "piercing" => 0,   "shell" => 0,   "elt" => 0.1,   "ko" => 10, },
	{ "name" => "L",   "divider" => 2.3,  "cut" => 0,     "impact" => 0,     "piercing" => 0.1, "shell" => 0,   "elt" => 0.1,   "ko" => 0,  },
	{ "name" => "GL",  "divider" => 2.3,  "cut" => 0.1,   "impact" => 0,     "piercing" => 0,   "shell" => 0.5, "elt" => 0.1,   "ko" => 0,  },
	{ "name" => "SA",  "divider" => 4.6,  "cut" => 0.1,   "impact" => 0,     "piercing" => 0,   "shell" => 0,   "elt" => 0.1,   "ko" => 0,  },
	{ "name" => "LBG", "divider" => 1.3,  "cut" => 0,     "impact" => 0,     "piercing" => 0,   "shell" => 0,   "elt" => 0,     "ko" => 0,  },
	{ "name" => "HBG", "divider" => 1.48, "cut" => 0,     "impact" => 0,     "piercing" => 0,   "shell" => 0,   "elt" => 0,     "ko" => 0,  },
	{ "name" => "B",   "divider" => 1.2,  "cut" => 0,     "impact" => 0,     "piercing" => 0,   "shell" => 0,   "elt" => 0,     "ko" => 0,  },
);


my %dcpy_profileid = (
		"Sword_and_Shield" 	=> 2,
		"Dual_Sword" 		=> 3,
		"Long_Sword" 		=> 1,
		"Great_Sword" 		=> 0,
		"Hammer" 			=> 4,
		"Hunting_Horn" 		=> 5,
		"Lance" 			=> 6,
		"Gunlance" 			=> 7,
		"Switch_Axe" 		=> 8
);

#print Dumper(@wclasses);
#exit;

my $wpn_class = $ARGV[3];

my $sta_nb_hits = 150;
my $sta_hit_factor = 0.25;
my $sta_regen_factor = 2;

my %status_factors = (
	"poison" =>  { "duration" => 0, "damage" => 1 },
	"sleep" =>   { "base" => 450, "duration" => 0 },
	"para" =>    { "duration" => 50 },
	"slime" =>   { "damage" => 1 },
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
my %sta_id = (6 => "poison", 7 => "para", 8 => "sleep", 9 => "slime");
my %phial_id = (1 => "power", 2 => "element", 3 => "dragon", 4 => "para", 5 => "poison", 6 => "exhaust");
my %tol_id = (1 => "poison", 2 => "sleep", 3 => "para", 4 => "stun", 5 => "exhaust", 6 => "slime");
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

my %base_buff = (
	"sharp" => 0,
	"awaken" => 0,
	"critical" => 0, # 0.10, 0.20, 0.30
	"atk_up" => 0, # 10, 15, 20
	"bombardier" => 1.0, # 1.2
	"element" => 1.0, # 1.10, 1.20
	"status" => 1.0, # 1.10, 1.20
	"fire" => 1.0,
	"water" => 1.0,
	"ice" => 1.0,
	"thunder" => 1.0,
	"dragon" => 1.0,
	"ko" => 1.0
);

my @buffs = (
	{ "sharp" => 1, "awaken" => 1 },
	{ "sharp" => 1, "atk_up" => 20 },
	{ "awaken" => 1, "atk_up" => 20 },
	{ "sharp" => 1, "critical" => 0.30 },
	{ "awaken" => 1, "critical" => 0.30 },
	{ "atk_up" => 20, "critical" => 0.30 },
	{ "status" => 1.2, "sharp" => 1 },
	{ "bombardier" => 1.2, "sharp" => 1 },
	{ "element" => 1.2, "sharp" => 1 },
	{ "status" => 1.2, "awaken" => 1 },
	{ "bombardier" => 1.2, "awaken" => 1 },
	{ "element" => 1.2, "awaken" => 1 },
	{ "status" => 1.2, "critical" => 0.30 },
	{ "status" => 1.2, "atk_up" => 20 },
	{ "element" => 1.2, "critical" => 0.30 },
	{ "element" => 1.2, "atk_up" => 20 },
	{ "bombardier" => 1.2, "critical" => 0.30 },
	{ "bombardier" => 1.2, "atk_up" => 20 }
	#{ "bombardier" => 1.3, "sharp" => 1, "awaken" => 1 }
);

open FILE, $ARGV[0] || die;
for (<FILE>) {
	chomp;
	next if /^#/;
	my @ln = split /\|/, $_, -1;

	my $raw_dmg = $ln[5] / $wclasses[$ln[2] - 1]->{"divider"};
	my $afi = ($ln[11] && $ln[11] > 0 ? $ln[11] : 0);

	my @shrp0 = @ln[16 .. 22];
	my @shrp1 = @ln[24 .. 30];

	my $rnsh = 4;
	my $rsharp = 0;
	my $resharp = 0;
	my $xnsh = 4;
	my $xsharp = 0;
	my $xesharp = 0;
	for (my $i = 6; $i >= 0; --$i) {
		if ($shrp1[$i] >= $xnsh) {
			$xsharp += $sharp_mod[$i] * $xnsh;
			$xesharp += $esharp_mod[$i] * $xnsh;
			$xnsh = 0;
		} else {
			$xsharp += $sharp_mod[$i] * $shrp1[$i];
			$xesharp += $esharp_mod[$i] * $shrp1[$i];
			$xnsh -= $shrp1[$i];
		}
		if ($shrp0[$i] >= $rnsh) {
			$rsharp += $sharp_mod[$i] * $rnsh;
			$resharp += $esharp_mod[$i] * $rnsh;
			$rnsh = 0;
		} else {
			$rsharp += $sharp_mod[$i] * $shrp0[$i];
			$resharp += $esharp_mod[$i] * $shrp0[$i];
			$rnsh -= $shrp0[$i];
		}
	}
	$xsharp /= 4;
	$xesharp /= 4;
	$rsharp /= 4;
	$resharp /= 4;

	my $elements = {};
	my $statuses = {};
	if ($elt_id{$ln[6]}) {
		if ($elt_id{$ln[8]}) {
			$elements->{$elt_id{$ln[6]}} = $ln[7] / 20;
			$elements->{$elt_id{$ln[8]}} = $ln[9] / 20;
		} else {
			$elements->{$elt_id{$ln[6]}} = $ln[7] / 10;
		}
	}
	if ($sta_id{$ln[6]}) {
		$statuses->{$sta_id{$ln[6]}} = $ln[7] / 10;
	}

	my $eltn = join('/', keys %{$elements}, keys %{$statuses}) || "raw";

	my $phial = "";
	if ($phial_id{$ln[32]}) {
		if ($ln[10] && ($statuses->{$phial_id{$ln[32]}} || $elements->{$phial_id{$ln[32]}})) {
			$phial = "awaken";
			$eltn = "{$eltn}" if ($ln[10]);
		} else {
			$eltn = "[$eltn]" if ($ln[10]);
			$phial = $phial_id{$ln[32]};
			$eltn = "$eltn,$phial"
		}
	} else {
		$eltn = "[$eltn]" if ($ln[10]);
	}

	push @wpns, {
		"name" => "$ln[0]",
		"raw" => $raw_dmg,
		"afi" => $afi,
		"elements" => $elements,
		"statuses" => $statuses,
		"type" => $eltn,
		"awakened" => $ln[10],
		"class" => $wclasses[$ln[2] - 1],
		"sharp0" => $rsharp,
		"sharp1" => $xsharp,
		"esharp0" => $resharp,
		"esharp1" => $xesharp,
		"phial" => $phial
	};
}
close FILE;

# print Dumper(@wpns);
# exit;

my %mons = ();

open FILE, $ARGV[2] || die;
for (<FILE>) {
	chomp;
	next if /^#/;
	my @ln = split /\|/, $_, -1;
	$mons{$ln[0]} ||= { "part" => {}, "tolerance" => {} };
	$mons{$ln[0]}{"tolerance"} ||= {};
	if ($ln[4]) {
		$mons{$ln[0]}{"tolerance"}{$tol_id{$ln[2]}} ||= {};
		my $mref_tol = $mons{$ln[0]}{"tolerance"}{$tol_id{$ln[2]}};
		$mref_tol->{"init"} = $ln[4] if $ln[4];
		$mref_tol->{"plus"} = $ln[5] if $ln[5];
		$mref_tol->{"max"} = $ln[6] if $ln[6];
		if ($ln[7] && $ln[7] =~ /(\d+)\/(\d+)/) {
			$mref_tol->{"regen_val"} = $1;
			$mref_tol->{"regen_tick"} = $2;
		}
		$mref_tol->{"duration"} = $ln[8] if $ln[8];
		$mref_tol->{"damage"} = $ln[9] if $ln[9];
	}
}
close FILE;

open FILE, $ARGV[1] || die;
my @colhead = ( "name", "hitzone", "id", "hzid", "cut", "impact", "bullet", "fire", "water", "ice", "thunder", "dragon", "stun", "exhaust" );
my %mons_acu = ();
for (<FILE>) {
	chomp;
	my @ln = split /\|/, $_, -1;
	my $have_alt = 0;
	for my $i (4 .. 11) {
		if ($ln[$i] =~ /(\d+)\s*\((\d+)\)/) {
			$have_alt = 1;
			last;
		}
	}
	$mons{$ln[0]} ||= { "part" => {}, "tolerance" => {} };
	my $mons_ref = $mons{$ln[0]};
	$mons_ref->{"part"}{$ln[1]} ||= {};
	$mons_ref->{"part"}{"$ln[1] (alt)"} ||= {} if ($have_alt);
	my $mref = $mons_ref->{"part"}{$ln[1]};
	my $mref_alt = $have_alt ? $mons_ref->{"part"}->{"$ln[1] (alt)"} : $mref;
	my $mref_tol = $mons_ref->{"tolerance"};

	$mons_acu{$ln[0]} ||= {};
	my $macuref = $mons_acu{$ln[0]};

	for my $i (4 .. 13) {
		my $colname = $colhead[$i];
		$macuref->{$colname} ||= [ 0, 0 ];
		if ($ln[$i] =~ /(\d+)\s*\((\d+)\)/) {
			$mref->{$colname} = $1;
			$macuref->{$colname}->[0] += $1;
			++$macuref->{$colname}->[1];
			$mref_alt->{$colname} = $2;
			$macuref->{$colname}->[0] += $2;
			++$macuref->{$colname}->[1];
		} elsif  ($ln[$i] =~ /(\d+)/) {
			$mref->{$colname} = $1;
			$macuref->{$colname}->[0] += $1;
			++$macuref->{$colname}->[1];
			if ($have_alt) {
				$mref_alt->{$colname} = $1;
				$macuref->{$colname}->[0] += $1;
				++$macuref->{$colname}->[1];
			}
		} else {
			$mref->{$colname} = 0;
			++$macuref->{$colname}->[1];
			if ($have_alt) {
				$mref_alt->{$colname} = 0;
				++$macuref->{$colname}->[1];
			}
		}
	}
}

for my $monster (keys %mons) {
	for my $dtype (keys %{$mons_acu{$monster}}) {
		if ($mons_acu{$monster}{$dtype}[1]) {
			$mons{$monster}{"part"}{"!avg"} ||= {};
			$mons{$monster}{"part"}{"!avg"}{$dtype} =
				$mons_acu{$monster}{$dtype}[0] / $mons_acu{$monster}{$dtype}[1];
		}
	}
	if ($wpn_class eq "H" || $wpn_class eq "HH") {
		$mons{$monster}{"part"}{"!avg"}{"stun"} = 50
	} else {
		$mons{$monster}{"part"}{"!avg"}{"stun"} = 20
	}
}
close FILE;

# print Dumper(%mons);
# exit;

sub get_sta_hits
{
	my $base_dmg = $_[0];
	my $tol = $_[1];
	my $factors = $_[2];

	my $sta_tdmg = $base_dmg;
	$sta_tdmg -= $sta_regen_factor * $tol->{"regen_val"} / $tol->{"regen_tick"} if ($tol->{"regen_tick"});
	return (0, 0) if ($sta_tdmg <= 0);
	$sta_tdmg *= $sta_nb_hits;

	my $sta_hits = 0;
	my $sta_next = $tol->{"init"};
	while ($sta_tdmg >= $sta_next) {
		$sta_tdmg -= $sta_next;
		$sta_next += $tol->{"plus"};
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
for my $wpn (@wpns) {
	next if ($wpn->{"class"}->{"name"} ne $wpn_class);

	my %buffkeys = ();
	for my $buff (@buffs) {
		my $newbuff = {};
		my $buffkey = "";
		for my $bk (sort keys %base_buff) {
			if (!defined $buff->{$bk}) {
				$newbuff->{$bk} = $base_buff{$bk};
			} elsif ($bk eq "awaken" && !$wpn->{"awakened"}) {
				$newbuff->{$bk} = $base_buff{$bk};
			} elsif ($need_awaken{$bk} && !$buff->{"awaken"} && $wpn->{"awakened"} && $wpn->{"phial"} ne "awaken") {
				$newbuff->{$bk} = $base_buff{$bk};
			} elsif  ($bk eq "sharp" && $wpn->{"sharp0"} == $wpn->{"sharp1"}) {
				$newbuff->{$bk} = $base_buff{$bk};
			} elsif ($bk eq "element" && !%{$wpn->{"elements"}}) {
				$newbuff->{$bk} = $base_buff{$bk};
			} elsif ($bk eq "status" && (!%{$wpn->{"statuses"}} || ($wpn->{"elements"}{"slime"}))) { # FIXME : impossible case with slime + element
				$newbuff->{$bk} = $base_buff{$bk};
			} elsif ($bk eq "bombardier" && !$wpn->{"statuses"}{"slime"}) {
				$newbuff->{$bk} = $base_buff{$bk};
			} elsif ($bk =~ /^(fire|water|ice|thunder|dragon)$/ && !$wpn->{"elements"}{$bk}) {
				$newbuff->{$bk} = $base_buff{$bk};
			} else {
				$newbuff->{$bk} = $buff->{$bk};
			}
			$buffkey .= "[$bk,$newbuff->{$bk}]";
		}

		unless ($buffkeys{$buffkey}) {
			my $buff = $bufflist{$buffkey} ||= $newbuff;
			my $afi = max(-1, min($wpn->{"afi"} + $buff->{"critical"}, 1));
			my $sharp = $buff->{"sharp"} ? $wpn->{"sharp1"} : $wpn->{"sharp0"};

			my $raw_atk = $wpn->{"raw"} + $buff->{"atk_up"} + $base_atk_up;
			$raw_atk *= $sharp * ($afi * 0.25 + 1) * $base_atk_factor;
			$raw_atk *= 1 + $phial_power_boost * $phial_rate if ($wpn->{"phial"} eq "power");

			my $ko_atk = $wpn->{"class"}{"ko"} + ($wpn->{"phial"} eq "exhaust" ? $phial_ko * $phial_rate : 0);
			$ko_atk *= $sharp * $buff->{"ko"};

			my $element_atk = {};
			my $status_atk = {};
			if ($buff->{"awaken"} || !$wpn->{"awakened"} || $wpn->{"phial"} eq "awaken") {
				my $aphial = $wpn->{"phial"} eq "awaken" && !$buff->{"awaken"} ? $phial_rate : 1;

				for my $element (keys %{$wpn->{"elements"}}) {
					my $esharp = $buff->{"sharp"} ? $wpn->{"esharp1"} : $wpn->{"esharp0"};
					my $elt_atk = $wpn->{"elements"}{$element} * $esharp;
					$elt_atk *= $aphial * max($buff->{$element}, $buff->{"element"});
					$elt_atk *= 1 + $phial_elt_boost * $phial_rate if ($wpn->{"phial"} eq "element");
					$element_atk->{$element} = $elt_atk * $wpn->{"class"}{"elt"};
				}
				for my $status (keys %{$wpn->{"statuses"}}) {
					my $sta_atk = ($afi * 0.25 + 1) * $aphial * $wpn->{"statuses"}{$status};
					$sta_atk *= $buff->{"status"} if ($status ne "slime");
					$sta_atk *= $buff->{"bombardier"} if ($status eq "slime");
					$status_atk->{$status} = $sta_atk * $wpn->{"class"}{"elt"} * $sta_hit_factor;
				}
			}

			my $wpn_buff = {
				"wpn" => $wpn,
				"buff" => $buff,
				"cut_atk" => $raw_atk * $wpn->{"class"}{"cut"},
				"impact_atk" => $raw_atk * $wpn->{"class"}{"impact"},
				"piercing_atk" => $raw_atk * $wpn->{"class"}{"piercing"},
				"element_atk" => $element_atk,
				"status_atk" => $status_atk,
				"ko_atk" => $ko_atk
			};
			push @wpn_buffs, $wpn_buff;
			$buffkeys{$buffkey} = 1;
		}
	}
}

# print Dumper(%bufflist);

my %glob_wpn_scores = ();
for my $monster (sort keys %mons) {
	my %wpn_buff_dmg = ();
	my %best_wpn_buff_hash = ();
	my @best_wpn_buff_list = ();
	my $mref_tol = $mons{$monster}{"tolerance"};
	my %wpn_scores = ();

	for my $dzone (sort keys %{$mons{$monster}{"part"}}) {
		my $mref = $mons{$monster}{"part"}{$dzone};

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
			$raw_dmg += $wpn_buff->{"piercing_atk"} * max($mref->{"cut"}, $mref->{"impact"} * 0.72)  / 100;
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

	print "----- $monster -----\n";
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
			printf " | %s : %s", $wpn->{"name"}, $wpn->{"type"};
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
	printf "%5.2f %6.2f : %s : %s\n", $w->[2], $w->[1] / (keys %mons), $w->[0]{"name"}, $w->[0]{"type"};
}

#print Dumper(%mons);
