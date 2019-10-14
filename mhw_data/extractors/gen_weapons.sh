#!/bin/sh

EXTRACTS_DIR="mhw-data-extracts"

./extract_kira_weap.pl \
"$EXTRACTS_DIR/kiranico_extracts/weapons/bow" \
"$EXTRACTS_DIR/kiranico_extracts/weapons/charge-blade" \
"$EXTRACTS_DIR/kiranico_extracts/weapons/dual-blades" \
"$EXTRACTS_DIR/kiranico_extracts/weapons/great-sword" \
"$EXTRACTS_DIR/kiranico_extracts/weapons/gunlance" \
"$EXTRACTS_DIR/kiranico_extracts/weapons/hammer" \
"$EXTRACTS_DIR/kiranico_extracts/weapons/heavy-bowgun" \
"$EXTRACTS_DIR/kiranico_extracts/weapons/hunting-horn" \
"$EXTRACTS_DIR/kiranico_extracts/weapons/insect-glaive" \
"$EXTRACTS_DIR/kiranico_extracts/weapons/lance" \
"$EXTRACTS_DIR/kiranico_extracts/weapons/light-bowgun" \
"$EXTRACTS_DIR/kiranico_extracts/weapons/long-sword" \
"$EXTRACTS_DIR/kiranico_extracts/weapons/switch-axe" \
"$EXTRACTS_DIR/kiranico_extracts/weapons/sword" > /tmp/kira_weap.xml

./extract_mhwg_weap.pl \
"$EXTRACTS_DIR/mhwg_extracts/weapons/4013.html" \
"$EXTRACTS_DIR/mhwg_extracts/weapons/4009.html" \
"$EXTRACTS_DIR/mhwg_extracts/weapons/4003.html" \
"$EXTRACTS_DIR/mhwg_extracts/weapons/4000.html" \
"$EXTRACTS_DIR/mhwg_extracts/weapons/4007.html" \
"$EXTRACTS_DIR/mhwg_extracts/weapons/4004.html" \
"$EXTRACTS_DIR/mhwg_extracts/weapons/4012.html" \
"$EXTRACTS_DIR/mhwg_extracts/weapons/4005.html" \
"$EXTRACTS_DIR/mhwg_extracts/weapons/4010.html" \
"$EXTRACTS_DIR/mhwg_extracts/weapons/4006.html" \
"$EXTRACTS_DIR/mhwg_extracts/weapons/4011.html" \
"$EXTRACTS_DIR/mhwg_extracts/weapons/4001.html" \
"$EXTRACTS_DIR/mhwg_extracts/weapons/4008.html" \
"$EXTRACTS_DIR/mhwg_extracts/weapons/4002.html" > /tmp/mhwg_weap.xml

./extract_mhwd_weap.pl \
"$EXTRACTS_DIR/MHWorldData/source_data/weapons/weapon_base.csv" \
"$EXTRACTS_DIR/MHWorldData/source_data/weapons/weapon_craft.csv" \
"$EXTRACTS_DIR/MHWorldData/source_data/weapons/weapon_bow_ext.csv" \
"$EXTRACTS_DIR/MHWorldData/source_data/weapons/weapon_ammo.csv" \
"$EXTRACTS_DIR/MHWorldData/source_data/weapons/weapon_sharpness.csv" > /tmp/mhwd_weap.xml

./extract_new_kira_weap.pl "$EXTRACTS_DIR/new_kiranico_extracts/weapons.html" > /tmp/new_kira_weap.xml

./extract_honey_weap.pl \
"$EXTRACTS_DIR/honey_extracts/weapon.json" \
"$EXTRACTS_DIR/honey_extracts/ib_kire.json" > /tmp/honey_weap.xml

./merge_weapons.pl translations.xml /tmp/mhwg_weap.xml /tmp/mhwd_weap.xml /tmp/new_kira_weap.xml /tmp/honey_weap.xml /tmp/kira_weap.xml
