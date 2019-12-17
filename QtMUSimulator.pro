#-------------------------------------------------
#
# Project created by QtCreator 2018-06-20T22:27:06
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtMUSimulator
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    muview.cpp \
    profile.cpp \
    simu/util.c \
    muconfiguration.cpp \
    simu/eth.c \
    simu/frame.c \
    simu/simu.c

HEADERS  += mainwindow.h \
    muview.h \
    globaltype.h \
    profile.h \
    simu/util.h \
    simu/all.h \
    simu/struct_frames92.h \
    simu/frame.h \
    muconfiguration.h \
    simu/eth.h

FORMS    += mainwindow.ui


INCLUDEPATH += ./simu/

LIBS += -lpcap
LIBS += -lpthread

QMAKE_CFLAGS += -D_LINUX
QMAKE_CFLAGS_DEBUG += -D_LINUX

DISTFILES += \
    MUConfig

RC_FILE += \
    MUSimulator.rc

OTHER_FILES += \
    MUSimulator.rc \
    MUSim.ico

RC_ICONS = MUSim.ico
