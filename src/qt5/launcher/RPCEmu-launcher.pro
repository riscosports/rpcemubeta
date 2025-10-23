#-------------------------------------------------
#
# Project created by QtCreator 2018-06-12T23:35:31
#
#-------------------------------------------------

# Add 'debug' to CONFIG to enable debugging symbols
CONFIG += c++11

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

win32 {
	TARGET = RPCEmu
	RC_ICONS = ../../win/rpcemu.ico
	CONFIG += console
} else {
	TARGET = rpcemu
}
TEMPLATE = app

# -Werror=switch
#	Ensures that using switch with enum requires every value to be handled
# -fno-common
#	Common symbols across object files will produce a link error
#	This is the default from GCC 10
#
QMAKE_CFLAGS   += -Werror=switch -fno-common
QMAKE_CXXFLAGS += -Werror=switch -fno-common

# Place exes in top level directory
DESTDIR = ../../..

# allow access to the main qt5 app header files
INCLUDEPATH += ../

# allow access to the rpcemu header files
INCLUDEPATH += ../../

SOURCES +=	main.cpp \
		mainwindow.cpp \
		machineimport.cpp \
		machinenew.cpp \
		machinesettings.cpp \
		lnat_edit_dialog.cpp \
		lnat_list_dialog.cpp \
		../about_dialog.cpp \
		../settings.cpp \
		../../rpcemu-models.c \
		../../rpcemu-nat-rules.c

HEADERS  +=	main.h \
		mainwindow.h \
		machineimport.h \
		machinenew.h \
		machinesettings.h \
		lnat_edit_dialog.h \
		lnat_list_dialog.h \
		../about_dialog.h \
		../../rpcemu.h \
		../../rpcemu-nat-rules.h

FORMS +=	mainwindow.ui \
		machineimport.ui \
		machinenew.ui \
		machinesettings.ui

RESOURCES +=	resources.qrc

CONFIG(debug, debug|release) {
	DEFINES += _DEBUG
	TARGET = $$join(TARGET, , , -debug)
}
