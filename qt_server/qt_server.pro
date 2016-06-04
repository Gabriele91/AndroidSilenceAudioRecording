#-------------------------------------------------
#
# Project created by QtCreator 2016-06-03T01:53:02
#
#-------------------------------------------------

QT += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qt_server
TEMPLATE = app


SOURCES += main.cpp\
        q_android_silence_audio_recording.cpp \
    q_settings.cpp \
    q_audio_output.cpp

HEADERS  += q_android_silence_audio_recording.h \
    rak_server.hpp \
    wave_riff.hpp \
    q_list_server_listener.h \
    q_settings.h \
    q_audio_server_listener.h \
    q_frame_double_click.h \
    q_audio_output.h

FORMS    += q_android_silence_audio_recording.ui \
    q_settings.ui

#dependencies headers
INCLUDEPATH += $$PWD/../server/dependencies/include
#dependencies libs path
CONFIG(debug, debug|release) {
    win32:QMAKE_LIBDIR  += $$PWD/../server/dependencies/lib/win32/debug
    mac:  QMAKE_LIBDIR  += $$PWD/../server/dependencies/lib/osx/debug
} else {
    win32:QMAKE_LIBDIR  += $$PWD/../server/dependencies/lib/win32/release
    mac:  QMAKE_LIBDIR  += $$PWD/../server/dependencies/lib/osx/release
}
#dependencies libs
win32:LIBS += silk_common.lib celt.lib opus.lib RakNetLibStatic.lib
mac:LIBS += -lopus -lRakNetLibStatic

RESOURCES += \
    qdarkstyle/style.qrc
