
CC = gcc
CFLAGS = -g -Wall -O2
CPPFLAGS = -MD -I../../include -I.

OBJS = lnb.o
SRCS = $(OBJS:.o=.c)

TARGET = lnb.o

$(TARGET): $(SRCS)

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o *.d $(TARGET)

-include $(wildcard *.d) dummy

