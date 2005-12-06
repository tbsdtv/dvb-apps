
CC = gcc
CFLAGS = -Wall -Wpointer-arith -g
CPPFLAGS = -MD -I. -I.. $(DEFS)
LDFLAGS = -r

OBJ =
INC = section.h \
      descriptor.h

TARGET = libatsc.a

.PHONY: all

all: $(TARGET)

$(TARGET): $(OBJ)
	$(LD) $(LDFLAGS) $(OBJ) -o $(TARGET)

install: $(LIB) $(INC)
	mkdir -p $(INSTDIR)/include/atsc
	cp -f $(INC) $(INSTDIR)/include/atsc

clean:
	rm -f *.d *.o $(TARGET) test *~ *#

-include $(wildcard *.d) dummy

