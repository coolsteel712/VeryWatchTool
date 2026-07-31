QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    about.cpp \
    bkground_helper.cpp \
    dialogiwflz.cpp \
    main.cpp \
    mainwindow.cpp \
    tool_utils.cpp

HEADERS += \
    about.h \
    bkground_helper.h \
    dialogiwflz.h \
    mainwindow.h \
    tool_utils.h

FORMS += \
    about.ui \
    bkground_helper.ui \
    dialogiwflz.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
