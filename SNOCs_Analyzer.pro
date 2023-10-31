#-------------------------------------------------
#
# Project created by QtCreator 2015-06-30T13:56:22
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SNOCsAnalyzer
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp \
    PerformanceAnalysis.cpp \
    PacketInfo.cpp \
    TrafficAnalysis.cpp

HEADERS  += MainWindow.h \
    DataReport.h \
    PerformanceAnalysis.h \
    PacketInfo.h \
    TrafficAnalysis.h

FORMS    += MainWindow.ui

RESOURCES += \
    resources.qrc
