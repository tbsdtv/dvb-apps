# Makefile for linuxtv.org dvb-apps/util/ca_zap

CPPFLAGS += -I../../lib
CFLAGS   ?= -g -O2 -Wall -W -Wshadow -Wpointer-arith -Wstrict-prototypes
LDFLAGS  += -L../../lib/libdvbapi -L../../lib/libdvbcfg -L../../lib/libdvben50221 -L../../lib/libucsi
LDLIBS   += -ldvbapi -ldvbcfg -ldvben50221 -lucsi -lpthread

objects  = ca_zap_hlci.o \
           ca_zap_llci.o

binaries = ca_zap

ifeq ($(prefix),)
prefix = /usr
endif

.PHONY: all clean install

all:		$(binaries)

$(binaries):	$(objects)

clean:
	rm -f $(objects) $(binaries)

install:
	mkdir -p $(prefix)/bin
	cp $(binaries) $(prefix)/bin
