
CC      = gcc
CFLAGS  = -g -O2 -Wall $(ARCH)
CPPFLAGS= -MD -I. -I../../include $(DEFS)
LFLAGS  =
DEFS = '-DSYSCONFDIR="$(sysconfdir)"'
bindir = /usr/local/bin
libdir = /usr/local/etc

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

install: install-bin install-scripts
install-bin: all
	test "$(bindir)/" != "/"
	mkdir -p $(DESTDIR)$(bindir)/
	install -m 755 $(TARGET) $(DESTDIR)$(bindir)/
NETSTART_SCRIPTS = net_start.pl net_start.sh
install-scripts: 
	test "$(libdir)/" != "/"
	mkdir -p $(DESTDIR)$(libdir)/dvb/
	for i in $(NETSTART_SCRIPTS) ; do cat $$i | sed -e \
	's|[.]/dvbnet |$(bindir)/dvbnet|' > $(DESTDIR)/$(libdir)/dvb/$$i \
	; chmod +x $(DESTDIR)/$(libdir)/dvb/$$i ; done

clean:
	rm -f $(TARGET) $(OBJS) version.h core* *~ *.d

-include $(wildcard *.d) dummy

