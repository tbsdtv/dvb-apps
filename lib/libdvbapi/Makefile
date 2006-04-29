# Makefile for linuxtv.org dvb-apps/lib/libdvbapi

CPPFLAGS += -I../../include -I../../lib
CFLAGS   ?= -g -O2 -Wall -W -Wshadow -Wpointer-arith -Wstrict-prototypes -fPIC

includes = diseqc.h   \
           dvbca.h    \
           dvbdemux.h \
           dvbfe.h    \
           dvbnet.h

objects  = diseqc.o   \
           dvbca.o    \
           dvbdemux.o \
           dvbfe.o    \
           dvbnet.o

sharedlib = libdvbapi.so
staticlib = libdvbapi.a

ifeq ($(prefix),)
prefix = /usr
endif

.PHONY: all clean install

all:		$(sharedlib) $(staticlib)

$(sharedlib):	$(objects)
	$(LD) -shared -o $(sharedlib) $(objects)

$(staticlib):	$(objects)
	$(AR) rcs $(staticlib) $(objects)

clean:
	rm -f $(objects) $(sharedlib) $(staticlib)

install:
	mkdir -p $(prefix)/include/libdvbapi
	cp $(includes) $(prefix)/include/libdvbapi
	mkdir -p $(prefix)/lib
	cp $(sharedlib) $(staticlib) $(prefix)/lib
