# Makefile for linuxtv.org dvb-apps/lib/libucsi/atsc

sub-includes = descriptor.h \
               section.h

sub-install += atsc

.PHONY: sub-error-atsc sub-install-atsc

sub-error-atsc:
	$(error You can't use this makefile directly.)

sub-install-atsc:
	mkdir -p $(prefix)/include/libucsi/atsc
	cp $(sub-includes) $(prefix)/include/libucsi/atsc
