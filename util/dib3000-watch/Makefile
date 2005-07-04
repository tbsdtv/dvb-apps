CC=gcc
TARGET=dib3000-watch
OBJECTS=dib3000-watch.o
CFLAGS ?= -Wall
LFLAGS=-lm

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LFLAGS) $(OBJECTS) -o $(TARGET)

clean:
	rm -f $(TARGET) $(OBJECTS) *~

dib-demod-watch.o: dib3000-watch.h dib-i2c.h dib3000.h
