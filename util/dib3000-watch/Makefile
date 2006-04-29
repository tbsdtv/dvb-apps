# Makefile for linuxtv.org dvb-apps/util/dib3000-watch

CFLAGS  ?= -g -O2 -Wall -W -Wshadow -Wpointer-arith -Wstrict-prototypes
LDLIBS += -lm

binaries = dib3000-watch

ifeq ($(prefix),)
prefix = /usr
endif

.PHONY: all clean install

all:		$(binaries)

clean:
	rm -f $(binaries)

install:
	mkdir -p $(prefix)/bin
	cp $(binaries) $(prefix)/bin
