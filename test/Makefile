# Makefile for linuxtv.org dvb-apps/test

CPPFLAGS += -I../include
CFLAGS   ?= -g -O2 -Wall -W -Wshadow -Wpointer-arith -Wstrict-prototypes

binaries = diseqc          \
           sendburst       \
           set22k          \
           setpid          \
           setvoltage      \
           test_av         \
           test_av_play    \
           test_dvr        \
           test_dvr_play   \
           test_pes        \
           test_sec_ne     \
           test_sections   \
           test_stc        \
           test_stillimage \
           test_tt         \
           test_vevent     \
           video

.PHONY: all clean install

all:		$(binaries)
	make -C libdvbcfg $@
	make -C libdvben50221 $@
	make -C libucsi $@

test_pes test_sec_ne test_sections test_tt: hex_dump.o

clean:
	rm -f hex_dump.o $(binaries)
	make -C libdvbcfg $@
	make -C libdvben50221 $@
	make -C libucsi $@

install:
