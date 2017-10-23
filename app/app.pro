TARGET = radio
QT = quickcontrols2

HEADERS = PresetDataObject.h qlibwindowmanager.h
SOURCES = main.cpp PresetDataObject.cpp qlibwindowmanager.cpp

CONFIG += link_pkgconfig
PKGCONFIG += windowmanager

RESOURCES += \
    radio.qrc \
    images/images.qrc

include(app.pri)
