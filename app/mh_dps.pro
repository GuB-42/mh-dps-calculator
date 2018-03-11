TEMPLATE = app
TARGET = mh_dps

CONFIG += debug

QT = core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
src/BuffGroup.cpp \
src/BuffWithCondition.cpp \
src/Build.cpp \
src/ConditionRatios.cpp \
src/Constants.cpp \
src/Damage.cpp \
src/DamageData.cpp \
src/FoldedBuffs.cpp \
src/Item.cpp \
src/MainData.cpp \
src/MhDpsApplication.cpp \
src/Monster.cpp \
src/NamedObject.cpp \
src/Profile.cpp \
src/Weapon.cpp \
src/enums.cpp \
src/main.cpp

HEADERS += \
src/BuffGroup.h \
src/BuffWithCondition.h \
src/Build.h \
src/ConditionRatios.h \
src/Constants.h \
src/Damage.h \
src/DamageData.h \
src/FoldedBuffs.h \
src/Item.h \
src/MainData.h \
src/MhDpsApplication.h \
src/Monster.h \
src/NamedObject.h \
src/Profile.h \
src/Weapon.h \
src/enums.h
