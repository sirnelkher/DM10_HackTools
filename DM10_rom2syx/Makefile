CC = gcc
LFLAGS =
CFLAGS =
LIBS =
INCLUDES = -Ilib
LDIR =
TARGET = rom2syx
SOURCES = rom2syx.c lib/crc.c

.PHONY: all

default: all

all:
	$(CC) $(CFLAGS) $(LFLAGS) $(INCLUDES) $(LIBS) -o $(TARGET) $(SOURCES) 

clean:
	rm $(TARGET)

