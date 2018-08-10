TEMPLATE = app
TARGET = mh_dps

CONFIG += debug

# QMAKE_CXXFLAGS_RELEASE += -g
# QMAKE_CFLAGS_RELEASE += -g
# QMAKE_LFLAGS_RELEASE =

QT = core gui xml concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
src/BuffGroup.cpp \
src/BuffWithCondition.cpp \
src/Build.cpp \
src/BuildWithDps.cpp \
src/ConditionRatios.cpp \
src/Constants.cpp \
src/Damage.cpp \
src/DamageData.cpp \
src/Dps.cpp \
src/FoldedBuffs.cpp \
src/Item.cpp \
src/MainData.cpp \
src/Monster.cpp \
src/MotionValue.cpp \
src/NamedObject.cpp \
src/Profile.cpp \
src/Target.cpp \
src/Weapon.cpp \
src/enums.cpp \
src/main.cpp \
src/gui/BuffGroupListModel.cpp \
src/gui/BuffListModel.cpp \
src/gui/ColumnMenu.cpp \
src/gui/ComputeDataDialog.cpp \
src/gui/ComputeDataModel.cpp \
src/gui/Computer.cpp \
src/gui/GuiElements.cpp \
src/gui/MainApplication.cpp \
src/gui/MainWindow.cpp \
src/gui/MonsterModeMenu.cpp \
src/gui/ResultTableModel.cpp

HEADERS += \
src/BuffGroup.h \
src/BuffWithCondition.h \
src/Build.h \
src/BuildWithDps.h \
src/ConditionRatios.h \
src/Constants.h \
src/Damage.h \
src/DamageData.h \
src/Dps.h \
src/FoldedBuffs.h \
src/Item.h \
src/MainData.h \
src/Monster.h \
src/MotionValue.h \
src/NamedObject.h \
src/Profile.h \
src/Target.h \
src/Weapon.h \
src/enums.h \
src/gui/BuffGroupListModel.h \
src/gui/BuffListModel.h \
src/gui/ColumnMenu.h \
src/gui/ComputeDataDialog.h \
src/gui/ComputeDataModel.h \
src/gui/Computer.h \
src/gui/GuiElements.h \
src/gui/MainApplication.h \
src/gui/MainWindow.h \
src/gui/MonsterModeMenu.h \
src/gui/ResultTableModel.h

FORMS += \
src/gui/ComputeDataDialog.ui \
src/gui/MainWindow.ui

TRANSLATIONS += \
src/translations/fr.ts

# TRANSLATIONS_FILES =

qtPrepareTool(LRELEASE, lrelease)
for(tsfile, TRANSLATIONS) {
#	qmfile = $$shadowed($$tsfile)
	qmfile = $$tsfile
	qmfile ~= s,.ts$,.qm,
	qmdir = $$dirname(qmfile)
#	!exists($$qmdir) {
#		mkpath($$qmdir)|error("Aborting.")
#	}
	command = $$LRELEASE -removeidentical $$tsfile -qm $$qmfile
	system($$command)|error("Failed to run: $$command")
#	TRANSLATIONS_FILES += $$qmfile
}

RESOURCES += src/gui/resources.qrc

RC_FILE = src/resources/winapp.rc
