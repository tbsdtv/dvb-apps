
CC = gcc
CFLAGS = -g -Wall -O2
CPPFLAGS = -MD -I../../include -I../lib
LFLAGS = -g -Wall
bindir = /usr/local/bin
datadir = /usr/local/share

OBJS = diseqc.o dump-zap.o dump-vdr.o scan.o ../lib/lnb.o
SRCS = $(OBJS:.o=.c)

TARGET = dvbscan

$(TARGET): $(OBJS)
	$(CC) $(LFLAGS) -o $(TARGET) $(OBJS)

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o *.d $(TARGET)

install: install-bin install-data
install-bin:
	test "$(bindir)/" != "/"
	mkdir -p $(DESTDIR)$(bindir)/
	for i in $(TARGETS) ; do cp $$i $(DESTDIR)$(bindir)/ ; done
install-data:
	test "$(datadir)/" != "/"
	mkdir -p $(DESTDIR)$(datadir)/dvb
	for d in dvb-* ; do test -d $$d || continue ; echo ..... $$d ..... \
	; mkdir -p $(DESTDIR)$(datadir)/dvb/$$d/ \
	; cp `find $$d -type f` $(DESTDIR)$(datadir)/dvb/$$d/ ; done

-include $(wildcard *.d) dummy

