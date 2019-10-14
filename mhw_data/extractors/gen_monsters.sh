#!/bin/sh

EXTRACTS_DIR="mhw-data-extracts"

./extract_mhwg_monster.pl \
"$EXTRACTS_DIR/mhwg_extracts/monsters/3200.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4320.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/3801.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4325.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4318.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/3800.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4255.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4215.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/3203.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/3803.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4257.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/3805.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4253.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4245.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4204.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4200.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4217.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4202.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4319.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/3804.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4216.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/3204.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4205.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4252.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4201.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4203.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4244.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4256.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4324.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/3806.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4254.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4358.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4321.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4361.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4363.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4364.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4365.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4369.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4372.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4368.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4366.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4367.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4373.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4379.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4380.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4381.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4382.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4385.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4387.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4388.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4389.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4391.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4393.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4401.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4402.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4403.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4404.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4405.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4406.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4407.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4408.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4409.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4413.html" \
"$EXTRACTS_DIR/mhwg_extracts/monsters/4414.html" > /tmp/mhwg_mons.xml

./extract_kira_monster.pl \
"$EXTRACTS_DIR/kiranico_extracts/monsters/anjanath" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/azure-rathalos" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/barroth" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/bazelgeuse" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/black-diablos" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/diablos" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/dodogama" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/great-girros" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/great-jagras" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/jyuratodus" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/kirin" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/kulu-ya-ku" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/kushala-daora" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/lavasioth" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/legiana" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/nergigante" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/odogaron" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/paolumu" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/pink-rathian" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/pukei-pukei" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/radobaan" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/rathalos" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/rathian" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/teostra" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/tobi-kadachi" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/tzitzi-ya-ku" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/uragaan" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/vaal-hazak" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/xenojiiva" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/zorah-magdaros" \
"$EXTRACTS_DIR/kiranico_extracts/monsters/deviljho" > /tmp/kira_mons.xml
#"$EXTRACTS_DIR/kiranico_extracts/monsters/kulve-taroth"

./merge_monsters.pl translations.xml /tmp/kira_mons.xml /tmp/mhwg_mons.xml
