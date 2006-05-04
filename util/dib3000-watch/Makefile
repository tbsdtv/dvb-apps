# Makefile for linuxtv.org dvb-apps/util/dib3000-watch

LDLIBS += -lm

binaries = dib3000-watch

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
