#-------------------------------------------------
#
# Project created by QtCreator 2013-10-17T07:05:16
#
#-------------------------------------------------

QT       += gui
CONFIG += qaxcontainer

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = upnp-test
TEMPLATE = app


SOURCES += main.cpp\
		mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

include(natportmapper/natportmapper.pri)



