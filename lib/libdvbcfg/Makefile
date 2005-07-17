
OBJS =  dvbcfg_util.o \
	dvbcfg_common.o \
        dvbcfg_source.o \
        dvbcfg_source_backend_file.o \
	dvbcfg_adapter.o \
        dvbcfg_adapter_backend_file.o \
	dvbcfg_diseqc.o \
        dvbcfg_diseqc_backend_file.o \
	dvbcfg_multiplex.o \
	dvbcfg_multiplex_backend_file.o \
	dvbcfg_vdrchannel.o \
	dvbcfg_zapchannel.o

INCLUDES = 	dvbcfg_common.h \
		dvbcfg_source.h \
		dvbcfg_source_backend_file.h \
		dvbcfg_adapter.h \
		dvbcfg_adapter_backend_file.h \
		dvbcfg_diseqc.h \
		dvbcfg_diseqc_backend_file.h \
		dvbcfg_multiplex.h \
		dvbcfg_multiplex_backend_file.h \
		dvbcfg_vdrchannel.h \
		dvbcfg_zapchannel.h

DVBCFG_DEFAULT_DIR = /tmp

CFLAGS ?= -O2 -g -Wall -Wstrict-prototypes -Wshadow -Wpointer-arith -DDVBCFG_DEFAULT_DIR=$(DVBCFG_DEFAULT_DIR)
CPPFLAGS = -I. -I../../include

LIB = libdvbcfg.a
RANLIB = ranlib
INSTDIR=/tmp

libdvbcfg.a: $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

test: $(LIB)
	make -C test

install: $(LIB) $(INCLUDES)
	mkdir -p $(INSTDIR)/lib
	mkdir -p $(INSTDIR)/include
	cp -f $(LIB) $(INSTDIR)/lib
	cp -f $(INCLUDES) $(INSTDIR)/include

clean:
	rm -f *~ *# *.o *.a
	make -C test clean

.PHONY: clean test
