
CC = gcc
CFLAGS = -g -Wall -O2
CPPFLAGS = -MD -I../../include -I../lib
LFLAGS = -g -Wall

OBJS = diseqc.o dump-zap.o dump-vdr.o scan.o ../lib/lnb.o
SRCS = $(OBJS:.o=.c)

TARGET = scan

$(TARGET): $(OBJS)
	$(CC) $(LFLAGS) -o $(TARGET) $(OBJS)

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o *.d $(TARGET)

-include $(wildcard *.d) dummy

