#!/bin/sh

./extract_kira_weap.pl \
kiranico_extracts/weapons/bow \
kiranico_extracts/weapons/charge-blade \
kiranico_extracts/weapons/dual-blades \
kiranico_extracts/weapons/great-sword \
kiranico_extracts/weapons/gunlance \
kiranico_extracts/weapons/hammer \
kiranico_extracts/weapons/heavy-bowgun \
kiranico_extracts/weapons/hunting-horn \
kiranico_extracts/weapons/insect-glaive \
kiranico_extracts/weapons/lance \
kiranico_extracts/weapons/light-bowgun \
kiranico_extracts/weapons/long-sword \
kiranico_extracts/weapons/switch-axe \
kiranico_extracts/weapons/sword > /tmp/kira_weap.xml

./extract_mhwg_weap.pl \
mhwg_extracts/weapons/4013.html \
mhwg_extracts/weapons/4009.html \
mhwg_extracts/weapons/4003.html \
mhwg_extracts/weapons/4000.html \
mhwg_extracts/weapons/4007.html \
mhwg_extracts/weapons/4004.html \
mhwg_extracts/weapons/4012.html \
mhwg_extracts/weapons/4005.html \
mhwg_extracts/weapons/4010.html \
mhwg_extracts/weapons/4006.html \
mhwg_extracts/weapons/4011.html \
mhwg_extracts/weapons/4001.html \
mhwg_extracts/weapons/4008.html \
mhwg_extracts/weapons/4002.html > /tmp/mhwg_weap.xml

./extract_mhwd_weap.pl \
MHWorldData/source_data/weapons/weapon_base.csv \
MHWorldData/source_data/weapons/weapon_craft.csv \
MHWorldData/source_data/weapons/weapon_bow_ext.csv \
MHWorldData/source_data/weapons/weapon_ammo.csv \
MHWorldData/source_data/weapons/weapon_sharpness.csv > /tmp/mhwd_weap.xml

./extract_new_kira_weap.pl kiranico_extracts/weapon_new_kira.html > /tmp/new_kira_weap.xml

./extract_honey_weap.pl honey_extracts/weapon_honey.json honey_extracts/weapon_honey_sharpness.json >  /tmp/honey_weap.xml

#./merge_weapons.pl translations.xml /tmp/mhwg_weap.xml /tmp/new_kira_weap.xml > /tmp/ice_weap_1.xml
#./merge_weapons.pl translations.xml /tmp/mhwg_weap.xml /tmp/honey_weap.xml > /tmp/ice_weap_2.xml
#./merge_weapons.pl translations.xml /tmp/new_kira_weap.xml /tmp/honey_weap.xml > /tmp/eng_ice_weap.xml
./merge_weapons.pl translations.xml /tmp/mhwg_weap.xml /tmp/mhwd_weap.xml /tmp/new_kira_weap.xml /tmp/kira_weap.xml /tmp/honey_weap.xml
