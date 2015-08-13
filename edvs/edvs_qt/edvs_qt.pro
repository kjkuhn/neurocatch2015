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
        mainwindow.cpp \
    tracker.cpp

HEADERS  += mainwindow.h \
    Edvs.h \
    tracker.h \
    settings.h

FORMS    += mainwindow.ui

INCLUDEPATH +=


QMAKE_CXXFLAGS += -std=c++11


LIBS += -L/usr/local/lib \
    -lboost_thread -lboost_system \
    -lopencv_features2d -lopencv_core \
    -lopencv_highgui \
    -lopencv_imgcodecs -lpthread

