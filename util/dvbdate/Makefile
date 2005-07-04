
CC      = gcc
CFLAGS  ?= -g -O2 -Wall
CFLAGS  += $(ARCH)
CPPFLAGS += -MD -I../../include $(DEFS)
LFLAGS  =

OBJS    = dvbdate.o
TARGET  = dvbdate
bindir  = /usr/local/bin

all: $(TARGET)

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LFLAGS)

install: all
	install -m 755 $(TARGET) $(DESTDIR)$(bindir)

clean:
	rm -f $(TARGET) $(OBJS) core* *~ *.d

-include $(wildcard *.d) dummy

