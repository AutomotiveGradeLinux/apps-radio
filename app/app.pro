TARGET = radio
QT = quickcontrols2 qml

HEADERS = PresetDataObject.h
SOURCES = main.cpp PresetDataObject.cpp

CONFIG += link_pkgconfig
PKGCONFIG += qlibsoundmanager qlibwindowmanager qlibhomescreen

RESOURCES += \
    radio.qrc \
    images/images.qrc

include(app.pri)
