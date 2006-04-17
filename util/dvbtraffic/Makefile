CFLAGS   ?= -g -O2 -Wall
CFLAGS   += $(ARCH)
CPPFLAGS += -MD -I../../include $(DEFS)
bindir = /usr/local/bin

all: dvbtraffic

dvbtraffic: dvbtraffic.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

clean:
	rm -f *.o *.d dvbtraffic

DVBTARGETS = dvbtraffic
install: install-bin
install-bin:
	test "$(bindir)/" != "/"
	mkdir -p $(DESTDIR)$(bindir)/
	for i in $(DVBTARGETS) ; do cp $$i $(DESTDIR)$(bindir)/$$i ; done
