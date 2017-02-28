TARGET = radio
QT = quickcontrols2

HEADERS = PresetDataObject.h
SOURCES = main.cpp PresetDataObject.cpp

RESOURCES += \
    radio.qrc \
    images/images.qrc

include(app.pri)
