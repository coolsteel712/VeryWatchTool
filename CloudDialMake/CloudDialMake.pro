QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    file_tool.cpp \
    main.cpp \
    mainwindow.cpp \
    make_watch_face.cpp \
    json_reader.cpp \
    json_value.cpp \
    json_writer.cpp \
    bitmap_tool.c \
    bitmap.c \
    lz4.c \
    fastlz_util.c \
    fastlz.c \
    debug.cpp \
    protocol_func_table.c

HEADERS += \
    bitmap_tool.h \
    file_tool.h \
    mainwindow.h \
    make_watch_face.h \
    bitmap.h \
    bitmap_font.h \
    blur.h \
    bmp_config.h \
    debug.h \
    error.h \
    fastlz.h \
    fastlz_util.h \
    include_help.h \
    json_tool.h \
    protocol_func_table.h \
    watch_face_rw_head.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
