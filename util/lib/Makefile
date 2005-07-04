
CC = gcc
CFLAGS ?= -g -Wall -O2
CFLAGS += $(ARCH)
CPPFLAGS += -MD -I../../include -I. $(DEFS)
bindir = /usr/local/bin

OBJS = lnb.o
SRCS = $(OBJS:.o=.c)

TARGET = lnb.o

$(TARGET): $(SRCS)

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o *.d $(TARGET)

install:

-include $(wildcard *.d) dummy

