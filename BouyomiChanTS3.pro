#-------------------------------------------------
#
# Project created by QtCreator 2016-11-09T21:27:50
#
#-------------------------------------------------

QT       += widgets network

TARGET = BouyomiChanTS3
TEMPLATE = lib

DEFINES += BouyomiChanTS3_LIBRARY

SOURCES += \
    dialog.cpp \
    globals.cpp \
    plugin.cpp

HEADERS += \
    teamlog/logtypes.h \
    teamspeak/clientlib_publicdefinitions.h \
    teamspeak/public_definitions.h \
    teamspeak/public_errors.h \
    teamspeak/public_errors_rare.h \
    teamspeak/public_rare_definitions.h \
    dialog.h \
    globals.h \
    plugin.h \
    plugin_definitions.h \
    ts3_functions.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

FORMS += \
    dialog.ui
