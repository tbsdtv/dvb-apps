
CC      = gcc
CFLAGS  = -g -O2 -Wall
CPPFLAGS= -MD -I../../include
LFLAGS  =

OBJS    = dvbdate.o
TARGET  = dvbdate
DESTDIR = /usr/local/bin/

all: $(TARGET)

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LFLAGS)

install: all
	install -m 755 $(TARGET) $(DESTDIR) 

clean:
	rm -f $(TARGET) $(OBJS) core* *~ *.d

-include $(wildcard *.d) dummy

