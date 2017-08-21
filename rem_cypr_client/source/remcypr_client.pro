#-------------------------------------------------
#
# Project created by QtCreator 2017-08-20T07:34:44
#
#-------------------------------------------------

QT       += core gui
LIBS += -lws2_32

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = remcypr_client
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    remcypr_client.cpp \
    att_thread.cpp

HEADERS  += mainwindow.h \
    remcypr_client.h \
    att_thread.h

FORMS    += mainwindow.ui

DISTFILES += \
    rem_cypr.png
