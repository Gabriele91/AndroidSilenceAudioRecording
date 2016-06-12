#-------------------------------------------------
#
# Project created by QtCreator 2016-06-03T01:53:02
#
#-------------------------------------------------


#add qt ev
QT += core gui multimedia printsupport

#flags
win32:DEFINES += _WINSOCKAPI_

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qt_server
TEMPLATE = app


SOURCES += main.cpp\
        q_android_silence_audio_recording.cpp \
    q_settings.cpp \
    q_options.cpp \
    q_rename.cpp \
    q_audio_server_listener.cpp

HEADERS  += q_android_silence_audio_recording.h \
    rak_server.hpp \
    wave_riff.hpp \
    q_list_server_listener.h \
    q_settings.h \
    q_audio_server_listener.h \
    q_frame_double_click.h \
    q_audio_player.h \
    q_options.h \
    q_rename.h \
    q_thread_utilities.h \
    string_utilities.h

FORMS    += q_android_silence_audio_recording.ui \
    q_settings.ui \
    q_options.ui \
    q_rename.ui

#qcustomplot
SOURCES += qcustomplot/qcustomplot.cpp
HEADERS += qcustomplot/qcustomplot.h

#dependencies headers
INCLUDEPATH += $$PWD/../server/dependencies/include
#dependencies libs path
CONFIG(debug, debug|release) {
    win32: {
        contains(QMAKE_HOST.arch, x86_64):{
            QMAKE_LIBDIR  += $$PWD/../server/dependencies/lib/x64/debug
        } else {
            QMAKE_LIBDIR  += $$PWD/../server/dependencies/lib/win32/debug
        }
    }
    mac:  QMAKE_LIBDIR  += $$PWD/../server/dependencies/lib/osx/debug
} else {
    win32: {
        contains(QMAKE_HOST.arch, x86_64):{
            QMAKE_LIBDIR  += $$PWD/../server/dependencies/lib/x64/release
        } else {
            QMAKE_LIBDIR  += $$PWD/../server/dependencies/lib/win32/release
        }
    }
    mac:  QMAKE_LIBDIR  += $$PWD/../server/dependencies/lib/osx/release
}
#dependencies libs
win32:LIBS += ws2_32.lib silk_common.lib celt.lib opus.lib RakNetLibStatic.lib
mac:LIBS += -lopus -lRakNetLibStatic

RESOURCES += \
    qdarkstyle/style.qrc
