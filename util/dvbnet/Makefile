
CC      = gcc
CFLAGS  = -g -O2 -Wall $(ARCH)
CPPFLAGS= -MD -I. -I../../include $(DEFS)
LFLAGS  =
bindir = /usr/local/bin
libdir = /usr/local/lib

OBJS    = dvbnet.o
TARGET  = dvbnet

all: version.h $(TARGET)

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

dvbnet.o: version.h

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
install-scripts:

clean:
	rm -f $(TARGET) $(OBJS) version.h core* *~ *.d

-include $(wildcard *.d) dummy

