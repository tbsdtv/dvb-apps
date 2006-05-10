# Makefile for linuxtv.org dvb-apps/lib/libdvbcfg

CPPFLAGS += -I..

includes = dvbcfg_zapchannel.h \
	   dvbcfg_sec.h

objects  = dvbcfg_zapchannel.o \
	   dvbcfg_sec.o \
	   dvbcfg_utils.o

sharedlib = libdvbcfg.so
staticlib = libdvbcfg.a

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
	mkdir -p $(prefix)/include/libdvbcfg
	cp $(includes) $(prefix)/include/libdvbcfg
	mkdir -p $(prefix)/lib
	cp $(sharedlib) $(staticlib) $(prefix)/lib

include ../../Make.rules
