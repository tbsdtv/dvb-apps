CC = gcc
CFLAGS = -Wall -g -O2
CPPFLAGS = -MD -I../../include -I../lib
LFLAGS = -Wall -g -O2
RM = rm -f
bindir = /usr/local/bin
datadir = /usr/local/share

TARGETS = szap tzap czap femon
OBJS = szap.o tzap.o czap.o femon.o

all: $(OBJS) $(TARGETS)
	@echo
	@echo "--------------------------------------------------------------------------------"
	@echo " please copy an appropriate channels.conf-XXX channel list for DVB-S/C/T"
	@echo
	@echo "   to ~/.szap/channels.conf"
	@echo "      ~/.czap/channels.conf"
	@echo "      ~/.tzap/channels.conf"
	@echo
	@echo " and then call ./szap for DVB-S, ./czap for DVB-C or ./tzap for DVB-T"
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
	test "$(datadir)/" != "/"
	mkdir -p $(DESTDIR)$(datadir)/dvb
	for i in channels.conf-* ; do cp $$i $(DESTDIR)$(datadir)/dvb ; done

-include $(wildcard *.d) dummy

