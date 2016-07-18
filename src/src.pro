#-------------------------------------------------
#
# Project created by QtCreator 2015-08-18T14:31:53
#
#-------------------------------------------------

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG(debug, debug|release) {
    TARGET = Qt5Flexd
} else {
    TARGET = Qt5Flex
}

TEMPLATE = lib

greaterThan(QT_MAJOR_VERSION, 4) {
        CONFIG += c++11
} else {
        *-g++* | *-clang* {
                QMAKE_CXXFLAGS += -std=c++11
        }
}

DEFINES += QT_FLEX_BUILD

QT += gui-private
QT += widgets-private

SOURCES += \
    QtDockGuider.cpp \
    QtDockSide.cpp \
    QtDockSite.cpp \
    QtDockWidget.cpp \
    QtFlexHelper.cpp \
    QtFlexManager.cpp \
    QtFlexStyle.cpp \
    QtFlexWidget.cpp

HEADERS += \
    QtDockGuider.h \
    QtDockSide.h \
    QtDockSite.h \
    QtDockWidget.h \
    QtFlexHelper.h \
    QtFlexManager.h \
    QtFlexStyle.h \
    QtFlexWidget.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

RESOURCES += \
    QtFlex.qrc
