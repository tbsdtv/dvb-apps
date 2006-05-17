# Makefile for linuxtv.org dvb-apps/util/dvbdate

CPPFLAGS += -I../../lib
LDFLAGS  += -L../../lib/libdvbapi -L../../lib/libucsi
LDLIBS   += -ldvbapi -lucsi -lpthread

binaries = dvbdate

ifeq ($(prefix),)
prefix = /usr
endif

.PHONY: all clean install

all: $(binaries)

clean::
	rm -f $(binaries)

install:
	mkdir -p $(prefix)/bin
	cp $(binaries) $(prefix)/bin

include ../../Make.rules
