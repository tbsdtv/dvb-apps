# Makefile for linuxtv.org dvb-apps/test/libdvbcfg

CPPFLAGS += -I../../lib
CFLAGS   += -g -O2 -Wall -W -Wshadow -Wpointer-arith -Wstrict-prototypes
LDLIBS   += ../../lib/libdvbcfg/libdvbcfg.a

binaries = dvbcfg_test

.PHONY: all clean install

all:		$(binaries)

clean:
	rm -f $(binaries)

install:
