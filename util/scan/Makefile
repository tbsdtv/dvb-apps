
CC = gcc
CFLAGS = -MD -g -Wall -O2 -I../../include -I../lib
LFLAGS = -g -Wall

OBJS = diseqc.o dump-zap.o dump-vdr.o scan.o ../lib/lnb.o
SRCS = $(OBJS:.o=.c)

TARGET = scan

$(TARGET): $(OBJS)
	$(CC) $(LFLAGS) -o $(TARGET) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o *.d $(TARGET)

-include $(wildcard *.d) dummy

