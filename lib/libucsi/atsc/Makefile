# Makefile for linuxtv.org dvb-apps/lib/libucsi/atsc

ifeq ($(MAKELEVEL),0)
$(error You can't use this makefile directly.)
endif

sub-includes = descriptor.h \
               section.h

sub-install += atsc

.PHONY: sub-install-atsc

sub-install-atsc:
	mkdir -p $(prefix)/include/libucsi/atsc
	cp $(sub-includes) $(prefix)/include/libucsi/atsc
