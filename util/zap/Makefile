# Makefile for linuxtv.org dvb-apps/util/ca_zap

CPPFLAGS += -I../../lib
LDFLAGS  += -L../../lib/libdvbapi -L../../lib/libdvbcfg -L../../lib/libdvben50221 -L../../lib/libucsi
LDLIBS   += -ldvbcfg -ldvben50221 -lucsi -lpthread -ldvbapi

objects  = zap_dvb.o \
           zap_ca.o

binaries = zap

ifeq ($(prefix),)
prefix = /usr
endif

.PHONY: all clean install

all: $(binaries)

$(binaries): $(objects)

clean::
	rm -f $(objects) $(binaries)

install:
	mkdir -p $(prefix)/bin
	cp $(binaries) $(prefix)/bin

include ../../Make.rules
