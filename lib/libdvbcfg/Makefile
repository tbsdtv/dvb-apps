# Makefile for linuxtv.org dvb-apps/lib/libdvbcfg

CPPFLAGS += -I../../lib
CFLAGS   += -g -O2 -Wall -W -Wshadow -Wpointer-arith -Wstrict-prototypes -fPIC
LDFLAGS  += -shared

includes = dvbcfg_zapchannel.h

objects  = dvbcfg_zapchannel.o

sharedlib = libdvbcfg.so
staticlib = libdvbcfg.a

ifeq ($(prefix),)
prefix = /usr
endif

.PHONY: all clean install

all:		$(sharedlib) $(staticlib)

$(sharedlib):	$(objects)
	$(CC) $(LDFLAGS) -o $(sharedlib) $(objects)

$(staticlib):	$(objects)
	$(AR) rcs $(staticlib) $(objects)

clean:
	rm -f $(objects) $(sharedlib) $(staticlib)

install:
	mkdir -p $(prefix)/include/libdvbcfg
	cp $(includes) $(prefix)/include/libdvbcfg
	mkdir -p $(prefix)/lib
	cp $(sharedlib) $(staticlib) $(prefix)/lib
