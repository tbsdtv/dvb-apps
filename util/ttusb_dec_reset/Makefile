# Makefile for linuxtv.org dvb-apps/util/ttusb_dec_reset

CFLAGS ?= -g -O2 -Wall -W -Wshadow -Wpointer-arith -Wstrict-prototypes
LDLIBS += -lusb

binaries = ttusb_dec_reset

ifeq ($(prefix),)
prefix = /usr
endif

.PHONY: all clean install

ifeq ($(ttusb_dec_reset),1)
all:		$(binaries)
else
all:		clean
endif

clean:
	rm -f $(binaries)

install:
ifneq ($(realpath $(binaries)),)
	mkdir -p $(prefix)/bin
	cp $(binaries) $(prefix)/bin
endif
