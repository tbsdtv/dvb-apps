# Makefile for linuxtv.org dvb-apps/util/szap

CPPFLAGS += -I../../include

binaries = azap  \
           czap  \
           femon \
           szap  \
           tzap

ifeq ($(prefix),)
prefix = /usr
endif

.PHONY: all clean install

all: $(binaries)

$(binaries): lnb.o

clean::
	rm -f lnb.o $(binaries)

install:
	mkdir -p $(prefix)/bin
	cp $(binaries) $(prefix)/bin

include ../../Make.rules
