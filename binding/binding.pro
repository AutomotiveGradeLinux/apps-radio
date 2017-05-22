TARGET = radio-binding

HEADERS = radio_impl.h radio_output.h rtl_fm.h convenience/convenience.h
SOURCES = radio-binding.c radio_output.c radio_impl_rtlsdr.c rtl_fm.c convenience/convenience.c

LIBS += -Wl,--version-script=$$PWD/export.map

CONFIG += link_pkgconfig
PKGCONFIG += json-c afb-daemon librtlsdr glib-2.0 libpulse-simple

include(binding.pri)
