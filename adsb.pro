#-------------------------------------------------
#
# Project created by QtCreator 2021-09-11T18:45:23
#
#-------------------------------------------------

QT       -= core gui

TARGET = /opt/vmbase/extensions/adsbPlugin
TEMPLATE = lib

DEFINES += VMPLUGINS_LIBRARY
DEFINES += QT_DEPRECATED_WARNINGS
LIBS += -lpthread -lusb-1.0

SOURCES += \
    libadsb.cpp \
    adsbplugin.cpp \
    librtlsdr/librtlsdr.c \
    librtlsdr/tuner_e4k.c \
    librtlsdr/tuner_fc0012.c \
    librtlsdr/tuner_fc0013.c \
    librtlsdr/tuner_fc2580.c \
    librtlsdr/tuner_r82xx.c \
    adsbframer.cpp \
    modesdecoder.cpp

HEADERS += \
    ConsumerProducer.h \
    vmplugins.h \
    vmsystem.h \
    plugin_factory.h \
    plugin_interface.h \
    vmtoolbox.h \
    vmtypes.h \
    adsbplugin.h \
    librtlsdr/reg_field.h \
    librtlsdr/rtl-sdr.h \
    librtlsdr/rtlsdr_i2c.h \
    librtlsdr/tuner_e4k.h \
    librtlsdr/tuner_fc0012.h \
    librtlsdr/tuner_fc0013.h \
    librtlsdr/tuner_fc2580.h \
    librtlsdr/tuner_r82xx.h \
    adsbframer.h \
    modesdecoder.h \
    json.hpp

DISTFILES += \
    example.js \
    example.js
