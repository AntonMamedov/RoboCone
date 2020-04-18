TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
INCLUDEPATH += I-/usr/include/opencv
LIBS += `pkg-config --cflags --libs opencv`
QMAKE_CXXFLAGS = -std=c++11
LIBS += -pthread
SOURCES += \
        ardoinocomport.cpp \
        configurator.cpp \
        http.cpp \
        main.cpp \
        objectepoll.cpp \
        objectsockets.cpp \
        sha1base64.cpp \
        socketpair.cpp \
        subprocess.cpp \
        vebcamstreamer.cpp \
        websocket.cpp

HEADERS += \
    WebConst.h \
    ardoinocomport.h \
    configurator.h \
    http.h \
    objectepoll.h \
    objectsockets.h \
    sha1base64.h \
    socketpair.h \
    subprocess.h \
    vebcamstreamer.h \
    websocket.h
