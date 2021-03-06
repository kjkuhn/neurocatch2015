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
    tracker.cpp \
    framemanager.cpp \
    spherocontroller.cpp

HEADERS  += mainwindow.h \
    Edvs.h \
    tracker.h \
    settings.h \
    framemanager.h \
    spherocontroller.h

FORMS    += mainwindow.ui

INCLUDEPATH +=


QMAKE_CXXFLAGS += -std=c++11


LIBS += -L/usr/local/lib \
    -lboost_thread -lboost_system \
    -lopencv_features2d -lopencv_core \
    -lopencv_highgui -lopencv_imgproc \
    -lopencv_calib3d -lopencv_xfeatures2d\
    -lopencv_imgcodecs -lpthread \
    -lbluetooth -lsphero

