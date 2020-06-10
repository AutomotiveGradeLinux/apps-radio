TARGET = radio
QT = quickcontrols2

HEADERS = PresetDataObject.h
SOURCES = main.cpp PresetDataObject.cpp

CONFIG += link_pkgconfig
PKGCONFIG += libhomescreen qlibwindowmanager qtappfw-radio

CONFIG(release, debug|release) {
    QMAKE_POST_LINK = $(STRIP) --strip-unneeded $(TARGET)
}

RESOURCES += \
    radio.qrc \
    images/images.qrc

include(app.pri)
