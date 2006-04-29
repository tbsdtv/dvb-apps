# Makefile for linuxtv.org dvb-apps/util/dvbnet

CPPFLAGS += -I../../include
CFLAGS   ?= -g -O2 -Wall -W -Wshadow -Wpointer-arith -Wstrict-prototypes

binaries = dvbnet

ifeq ($(prefix),)
prefix = /usr
endif

CFLAGS += -DVERSION_INFO=\"1.1.0-TVF\" # FIXME

.PHONY: all clean install

all:		$(binaries)

clean:
	rm -f $(binaries)

install:
	mkdir -p $(prefix)/bin
	cp $(binaries) $(prefix)/bin
