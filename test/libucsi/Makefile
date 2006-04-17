# Makefile for linuxtv.org dvb-apps/test/libucsi

CPPFLAGS += -I../../lib
CFLAGS   += -g -O2 -Wall -Wextra -Wshadow -Wpointer-arith -Wstrict-prototypes
LDLIBS   += ../../lib/libdvbapi/libdvbapi.a ../../lib/libdvbcfg/libdvbcfg.a ../../lib/libucsi/libucsi.a

binaries = testucsi

.PHONY: all clean install

all:		$(binaries)

clean:
	rm -f $(binaries)

install:
