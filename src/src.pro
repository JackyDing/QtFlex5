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

QMAKE_CXXFLAGS += -std=c++11 -stdlib=libc++

DEFINES += QT_FLEX_BUILD

INCLUDEPATH += "../../../../libs/Qt/5.5.0/5.5/msvc2013_64/include/QtCore/5.5.0/QtCore"
INCLUDEPATH += "../../../../libs/Qt/5.5.0/5.5/msvc2013_64/include/QtGui/5.5.0/QtGui"
INCLUDEPATH += "../../../../libs/Qt/5.5.0/5.5/msvc2013_64/include/QtWidgets/ß5.5.0/QtWidgets"
INCLUDEPATH += "../../../../libs/Qt/5.5.0/5.5/msvc2013_64/include/QtWidgets"

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
