# Makefile for linuxtv.org dvb-apps/lib/libucsi/atsc

objects += atsc/stt_section.o           \
	   atsc/mgt_section.o           \
	   atsc/tvct_section.o           \
	   atsc/cvct_section.o           \
	   atsc/rrt_section.o           \
	   atsc/eit_section.o           \
	   atsc/ett_section.o           \
	   atsc/dcct_section.o           \
	   atsc/dccsct_section.o           \
	   atsc/atsc_huffman.o			\
	   atsc/types.o           \

sub-includes = descriptor.h \
               section.h \
               types.h \
               stt_section.h \
               mgt_section.h \
               tvct_section.h \
               cvct_section.h \
               rrt_section.h \
               eit_section.h \
               ett_section.h \
               dcct_section.h \
               dccsct_section.h \
               caption_service_descriptor.h \
	       content_advisory_descriptor.h \
	       extended_channel_name_descriptor.h \
	       service_location_descriptor.h \
	       time_shifted_service_descriptor.h \
	       component_name_descriptor.h \
	       dcc_departing_request_descriptor.h \
	       dcc_arriving_request_descriptor.h \
	       rc_descriptor.h \

sub-install += atsc

.PHONY: sub-error-atsc sub-install-atsc

sub-error-atsc:
	$(error You can't use this makefile directly.)

sub-install-atsc:
	mkdir -p $(prefix)/include/libucsi/atsc
	cp $(sub-includes) $(prefix)/include/libucsi/atsc
