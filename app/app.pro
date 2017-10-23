TARGET = radio
QT = quickcontrols2

HEADERS = PresetDataObject.h qlibwindowmanager.h qlibsoundmanager.h
SOURCES = main.cpp PresetDataObject.cpp qlibwindowmanager.cpp qlibsoundmanager.cpp

CONFIG += link_pkgconfig
PKGCONFIG += soundmanager windowmanager

RESOURCES += \
    radio.qrc \
    images/images.qrc

include(app.pri)
