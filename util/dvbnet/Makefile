
CC      = gcc
CFLAGS  = -g -O2 -Wall
CPPFLAGS= -MD -I. -I../../include
LFLAGS  =
bindir = /usr/local/bin
sysconfdir = /usr/local/etc

OBJS    = dvbnet.o
TARGET  = dvbnet

all: version.h $(TARGET)

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LFLAGS)

version.h:
	printf '#define VERSION_INFO "%s (Build %s)"\n' \
		"`cat $@.in`" "`date +'%a %b %d %X %Y'`" > $@

install: install-bin
install-bin: all
	test "$(bindir)/" != "/"
	mkdir -p $(DESTDIR)$(bindir)/
	install -m 755 $(TARGET) $(DESTDIR)$(bindir)/
NETSTART_SCRIPTS = net_start.pl net_start.sh
install-etc: 
	test "$(sysconfdir)/" != "/"
	mkdir -p $(DESTDIR)$(sysconfdir)/dvb/
	for i in $(NETSTART_SCRIPTS) ; do cat $$i | sed -e \
	's|[.]/dvbnet |$(bindir)/dvbnet|' > $(DESTDIR)/$(sysconfdir)/dvb/$$i \
	; chmod +x $(DESTDIR)/$(sysconfdir)/dvb/$$i ; done

clean:
	rm -f $(TARGET) $(OBJS) version.h core* *~ *.d

-include $(wildcard *.d) dummy

