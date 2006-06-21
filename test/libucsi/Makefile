# Makefile for linuxtv.org dvb-apps/test/libucsi

CPPFLAGS += -I../../lib
LDLIBS   += ../../lib/libdvbapi/libdvbapi.a ../../lib/libdvbcfg/libdvbcfg.a ../../lib/libucsi/libucsi.a
LDFLAGS  += -static 

binaries = testucsi

.PHONY: all clean install

all: $(binaries)

clean::
	rm -f $(binaries)

install:

include ../../Make.rules
