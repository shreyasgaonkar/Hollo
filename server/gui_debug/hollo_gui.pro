######################################################################
# Automatically generated by qmake (3.0) Thu Nov 12 21:02:37 2015
######################################################################

TEMPLATE = app
TARGET = hollo_gui
INCLUDEPATH += .

QT += widgets

CONFIG += -std=c++11
QMAKE_CXXFLAGS += -std=c++11

LIBS += -lboost_system -lboost_thread -pthread

# Input
HEADERS += chat_client.hpp main_window.h user.hpp chat_message.hpp definitions.h

FORMS += main_window.ui

SOURCES += main.cpp main_window.cpp
