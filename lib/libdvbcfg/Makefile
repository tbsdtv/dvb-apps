# Makefile for linuxtv.org dvb-apps/lib/libdvbcfg

CPPFLAGS += -I../../lib
CFLAGS   += -g -O2 -Wall -W -Wshadow -Wpointer-arith -Wstrict-prototypes -fPIC
LDFLAGS  += -shared

includes = dvbcfg_adapter.h                \
           dvbcfg_adapter_backend_file.h   \
           dvbcfg_common.h                 \
           dvbcfg_diseqc.h                 \
           dvbcfg_diseqc_backend_file.h    \
           dvbcfg_multiplex.h              \
           dvbcfg_multiplex_backend_file.h \
           dvbcfg_seed.h                   \
           dvbcfg_seed_backend_file.h      \
           dvbcfg_source.h                 \
           dvbcfg_source_backend_file.h    \
           dvbcfg_util.h                   \
           dvbcfg_vdrchannel.h             \
           dvbcfg_zapchannel.h

objects  = dvbcfg_adapter.o                \
           dvbcfg_adapter_backend_file.o   \
           dvbcfg_common.o                 \
           dvbcfg_diseqc.o                 \
           dvbcfg_diseqc_backend_file.o    \
           dvbcfg_multiplex.o              \
           dvbcfg_multiplex_backend_file.o \
           dvbcfg_seed.o                   \
           dvbcfg_seed_backend_file.o      \
           dvbcfg_source.o                 \
           dvbcfg_source_backend_file.o    \
           dvbcfg_util.o                   \
           dvbcfg_vdrchannel.o             \
           dvbcfg_zapchannel.o

sharedlib = libdvbcfg.so
staticlib = libdvbcfg.a

ifeq ($(prefix),)
prefix = /usr
endif

CFLAGS += -DDVBCFG_DEFAULT_DIR=\"/etc/dvb\" # FIXME

.PHONY: all clean install

all:		$(sharedlib) $(staticlib)

$(sharedlib):	$(objects)
	$(CC) $(LDFLAGS) -o $(sharedlib) $(objects)

$(staticlib):	$(objects)
	$(AR) rcs $(staticlib) $(objects)

clean:
	rm -f $(objects) $(sharedlib) $(staticlib)

install:
	mkdir -p $(prefix)/include/libdvbcfg
	cp $(includes) $(prefix)/include/libdvbcfg
	mkdir -p $(prefix)/lib
	cp $(sharedlib) $(staticlib) $(prefix)/lib
