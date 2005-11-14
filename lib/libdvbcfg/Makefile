
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
        dvbcfg_seed.o \
        dvbcfg_seed_backend_file.o \
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
		dvbcfg_seed.h \
		dvbcfg_seed_backend_file.h \
		dvbcfg_vdrchannel.h \
		dvbcfg_zapchannel.h

DVBCFG_DEFAULT_DIR = /etc/dvb

CFLAGS ?= -fPIC -O2 -g -Wall -Wstrict-prototypes -Wshadow -Wpointer-arith -DDVBCFG_DEFAULT_DIR=\"$(DVBCFG_DEFAULT_DIR)\"
CPPFLAGS = -I. -I../libdvbapi
LDFLAGS = -shared -lc

STATICLIB = libdvbcfg.a
SHAREDLIB = libdvbcfg.so
RANLIB = ranlib
STRIP = strip
INSTDIR=/usr/local

all: symlink $(STATICLIB) $(SHAREDLIB)

symlink:
	@rm -f dvbcfg
	@ln -s . dvbcfg

$(STATICLIB): $(OBJS)
	$(AR) rc $(STATICLIB) $(OBJS)
	$(RANLIB) $(STATICLIB)

$(SHAREDLIB): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(SHAREDLIB)

test: $(LIB)
	make -C test

install: $(STATICLIB) $(SHAREDLIB) $(INCLUDES)
	mkdir -p $(INSTDIR)/lib
	mkdir -p $(INSTDIR)/include/dvbcfg
	cp -f $(STATICLIB) $(INSTDIR)/lib
	cp -f $(SHAREDLIB) $(INSTDIR)/lib
	$(STRIP) $(INSTDIR)/lib/$(SHAREDLIB)
	cp -f $(INCLUDES) $(INSTDIR)/include/dvbcfg

clean:
	rm -f *~ *# *.o *.a *.so dvbcfg
	make -C test clean

.PHONY: clean test all
