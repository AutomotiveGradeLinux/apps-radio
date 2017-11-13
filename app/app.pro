TARGET = radio
QT = quickcontrols2 qml

HEADERS = PresetDataObject.h qlibsoundmanager.h
SOURCES = main.cpp PresetDataObject.cpp qlibsoundmanager.cpp

CONFIG += link_pkgconfig
PKGCONFIG += libsoundmanager qlibwindowmanager qlibhomescreen

RESOURCES += \
    radio.qrc \
    images/images.qrc

include(app.pri)
