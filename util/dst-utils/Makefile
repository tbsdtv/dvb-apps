# Makefile for linuxtv.org dvb-apps/util/dst-utils

CPPFLAGS += -I../../include -I../../lib

binaries = dst_test

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
