# Makefile for linuxtv.org dvb-apps/test/libdvbcfg

CPPFLAGS += -I../../lib
LDLIBS   += ../../lib/libdvbcfg/libdvbcfg.a

binaries = dvbcfg_test

.PHONY: all clean install

all: $(binaries)

clean::
	rm -f $(binaries)

install:

include ../../Make.rules
