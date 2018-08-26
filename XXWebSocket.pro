TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += "tools"
LIBS += -lpthread libwsock32 libws2_32


SOURCES += main.cpp \
    xxWS/eventmanager.cpp \
    xxWS/httprequest.cpp \
    xxWS/httpresponse.cpp \
    xxWS/mbuffer.cpp \
    xxWS/websocket.cpp \
    xxWS/xxserver.cpp \
    xxWS/xxsocket.cpp \
    tools/mbedtls/base64.c \
    tools/mbedtls/sha1.c

HEADERS += \
    tools/mbedtls/base64.h \
    tools/mbedtls/check_config.h \
    tools/mbedtls/config.h \
    tools/mbedtls/platform.h \
    tools/mbedtls/platform_time.h \
    tools/mbedtls/sha1.h \
    xxWS/event.h \
    xxWS/eventmanager.h \
    xxWS/httprequest.h \
    xxWS/httpresponse.h \
    xxWS/mbuffer.h \
    xxWS/net_common.h \
    xxWS/websocket.h \
    xxWS/xxserver.h \
    xxWS/xxsocket.h
