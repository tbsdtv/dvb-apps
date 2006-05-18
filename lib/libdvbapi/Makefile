# Makefile for linuxtv.org dvb-apps/lib/libdvbapi

CPPFLAGS += -I../../include -I..

includes = diseqc.h   \
           dvbca.h    \
           dvbdemux.h \
           dvbaudio.h \
           dvbvideo.h \
           dvbfe.h    \
           dvbnet.h

objects  = dvbca.o    \
           dvbdemux.o \
           dvbfe.o    \
           dvbaudio.o \
           dvbvideo.o \
           dvbnet.o

sharedlib = libdvbapi.so
staticlib = libdvbapi.a

ifeq ($(prefix),)
prefix = /usr
endif

.PHONY: all clean install

all: $(sharedlib) $(staticlib)

$(sharedlib): $(objects)

$(staticlib): $(objects)

clean::
	rm -f $(objects) $(sharedlib) $(staticlib)

install:
	mkdir -p $(prefix)/include/libdvbapi
	cp $(includes) $(prefix)/include/libdvbapi
	mkdir -p $(prefix)/lib
	cp $(sharedlib) $(staticlib) $(prefix)/lib

include ../../Make.rules
