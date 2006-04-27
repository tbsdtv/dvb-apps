# Makefile for Linux DVB API Version 3 test programs

CC = gcc
CFLAGS ?= -g -O2 -W -Wall
CFLAGS += -D__user= $(ARCH)
CPPFLAGS += -I../include $(DEFS)

TARGETS = \
	diseqc		\
	set22k		\
	sendburst	\
	setvoltage	\
	setpid		\
	video		\
	test_sections	\
	test_sec_ne	\
	test_pes	\
	test_dvr	\
	test_dvr_play	\
	test_tt		\
	test_av		\
	test_av_play	\
	test_vevent	\
	test_stc	\
	test_stillimage

#	test		\
#	test_audio	\
#	test_front	\
#	test_switch	\
#	test_video	\

all: $(TARGETS)
	make -C libdvbcfg
	make -C libdvben50221

test_sections test_sec_ne test_pes test_tt: hex_dump.o

clean:
	rm -f $(TARGETS) *.o
	make -C libdvbcfg clean
	make -C libdvben50221 clean

install:

