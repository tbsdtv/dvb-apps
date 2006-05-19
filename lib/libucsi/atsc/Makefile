# Makefile for linuxtv.org dvb-apps/lib/libucsi/atsc

objects += atsc/stt_section.o           \
	   atsc/mgt_section.o           \
	   atsc/tvct_section.o           \
	   atsc/cvct_section.o           \

sub-includes = descriptor.h \
               section.h \
               stt_section.h \
               mgt_section.h \
               tvct_section.h \
               cvct_section.h

sub-install += atsc

.PHONY: sub-error-atsc sub-install-atsc

sub-error-atsc:
	$(error You can't use this makefile directly.)

sub-install-atsc:
	mkdir -p $(prefix)/include/libucsi/atsc
	cp $(sub-includes) $(prefix)/include/libucsi/atsc
