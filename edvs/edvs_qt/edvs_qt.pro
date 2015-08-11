#-------------------------------------------------
#
# Project created by QtCreator 2015-08-03T16:34:22
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = evds_qt
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h \
    Edvs.h

FORMS    += mainwindow.ui

INCLUDEPATH +=


LIBS += -lboost_thread -lboost_system
