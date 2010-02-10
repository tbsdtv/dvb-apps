# Makefile for linuxtv.org dvb-apps/util/szap

objects  = lnb.o util.o

binaries = azap  \
           czap  \
           szap  \
           tzap

inst_bin = $(binaries)

.PHONY: all

all: $(binaries)

$(binaries): $(objects)

include ../../Make.rules
