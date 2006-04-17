CC	= gcc
RM	= rm -f
CFLAGS  ?= -g -Wall -O2 
CFLAGS  += $(ARCH)
CPPFLAGS += $(DEFS)
LFLAGS	= -g -Wall
LDFLAGS = -lusb
bindir  = /usr/local/bin

OBJS	= ttusb_dec_reset.o
TARGET	= ttusb_dec_reset

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o $(TARGET)

install: $(TARGET)
	install -m 755 $(TARGET) $(DESTDIR)$(bindir)
