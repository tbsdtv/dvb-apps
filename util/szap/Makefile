CC = gcc
CFLAGS = -Wall -g -O2 $(ARCH)
CPPFLAGS = -MD -I../../include -I../lib $(DEFS)
LFLAGS = -Wall -g -O2
RM = rm -f
DEFS = '-DDATADIR="$(datadir)"'
bindir = /usr/local/bin
datadir = /usr/local/share

TARGETS = azap szap tzap czap femon
OBJS = azap.o szap.o tzap.o czap.o femon.o

all: $(OBJS) $(TARGETS)
	@echo
	@echo "--------------------------------------------------------------------------------"
	@echo " please copy an appropriate channels-conf/*/XXX channel list for DVB-S/C/T or ATSC"
	@echo
	@echo "   to ~/.szap/channels.conf"
	@echo "      ~/.czap/channels.conf"
	@echo "      ~/.tzap/channels.conf"
	@echo "      ~/.azap/channels.conf"
	@echo
	@echo " and then call ./szap for DVB-S, ./czap for DVB-C or ./tzap for DVB-T or ./azap for ATSC"
	@echo "--------------------------------------------------------------------------------"
	@echo

szap: szap.o ../lib/lnb.o
	$(CC) $(LFLAGS) -o szap szap.o ../lib/lnb.o

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

.o:
	$(CC) $(LFLAGS) -o $@ $<

clean:
	$(RM) $(TARGETS) core* *.o *.d .depend

install: install-bin install-data
install-bin:
	test "$(bindir)/" != "/"
	mkdir -p $(DESTDIR)$(bindir)/
	for i in $(TARGETS) ; do cp $$i $(DESTDIR)$(bindir)/ ; done
install-data:

-include $(wildcard *.d) dummy

