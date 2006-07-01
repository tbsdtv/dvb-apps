# Makefile for linuxtv.org dvb-apps/util/av7110_loadkeys

binaries = av7110_loadkeys \
           evtest

inst_bin = $(binaries)

removing = input_keynames.h

.PHONY: all

all: $(binaries)

$(binaries): input_keynames.h

input_keynames.h:
	$(SHELL) generate-keynames.sh $@

include ../../Make.rules
