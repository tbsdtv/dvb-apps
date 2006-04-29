# Makefile for linuxtv.org dvb-apps/util/av7110_loadkeys

CFLAGS ?= -g -O2 -Wall -W -Wshadow -Wpointer-arith -Wstrict-prototypes

binaries = av7110_loadkeys \
           evtest

ifeq ($(prefix),)
prefix = /usr
endif

.PHONY: all clean install

all:		$(binaries)

$(binaries):	input_keynames.h

input_keynames.h:
	$(SHELL) generate-keynames.sh $@

clean:
	rm -f input_keynames.h $(binaries)

install:
	mkdir -p $(prefix)/bin
	cp $(binaries) $(prefix)/bin
