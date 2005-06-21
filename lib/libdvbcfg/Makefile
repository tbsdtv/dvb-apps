
OBJS =  dvbcfg_util.o \
        dvbcfg_source.o \
	dvbcfg_diseqc.o \
	dvbcfg_vdrchannel.o \
	dvbcfg_zapchannel.o \
	dvbcfg_adapter.o \
	dvbcfg_common.o

INCLUDES = 	dvbcfg_source.h \
		dvbcfg_diseqc.h \
		dvbcfg_vdrchannel.h \
		dvbcfg_zapchannel.h \
		dvbcfg_adapter.h \
		dvbcfg_common.h

CFLAGS = -I.

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
