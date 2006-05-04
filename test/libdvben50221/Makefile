# Makefile for linuxtv.org dvb-apps/test/libdvben50221

CPPFLAGS += -I../../lib
LDLIBS   += ../../lib/libdvbapi/libdvbapi.a ../../lib/libdvben50221/libdvben50221.a ../../lib/libucsi/libucsi.a -lpthread

binaries = test-app       \
           test-session   \
           test-transport

.PHONY: all clean install

all: $(binaries)

clean::
	rm -f $(binaries)

install:

include ../../Make.rules
