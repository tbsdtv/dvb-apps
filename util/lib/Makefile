
CC = gcc
CFLAGS = -MD -g -Wall -O2 -I../../include -I.
LFLAGS = -g -Wall

OBJS = lnb.o
SRCS = $(OBJS:.o=.c)

TARGET = lnb.o

$(TARGET): $(SRCS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o *.d $(TARGET)

-include $(wildcard *.d) dummy

