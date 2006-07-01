# Makefile for linuxtv.org dvb-apps/util/szap

objects  = lnb.o

binaries = azap  \
           czap  \
           szap  \
           tzap

inst_bin = $(binaries)

CPPFLAGS += -I../../include

.PHONY: all

all: $(binaries)

$(binaries): $(objects)

include ../../Make.rules
