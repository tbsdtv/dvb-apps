
CC      = gcc
CFLAGS  = -g -O2 -MD -Wall -I. -I../../include
LFLAGS  =

OBJS    = dvbnet.o
TARGET  = dvbnet
DESTDIR = /usr/local/bin/

all: version.h $(TARGET)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LFLAGS)

version.h:
	printf '#define VERSION_INFO "%s (Build %s)"\n' \
		"`cat $@.in`" "`date +'%a %b %d %X %Y'`" > $@

install: all
	install -m 755 $(TARGET) $(DESTDIR) 

clean:
	rm -f $(TARGET) $(OBJS) version.h core* *~ *.d

-include $(wildcard *.d) dummy

