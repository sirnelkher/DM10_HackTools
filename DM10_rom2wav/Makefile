CC = gcc
LFLAGS =
CFLAGS = -Wimplicit-function-declaration
LIBS = -lm
INCLUDES = -Ilib
LDIR =
TARGET = rom2wav
SOURCES = lib/cJSON.c rom2wav.c

.PHONY: all

default: all

all:
	$(CC) $(CFLAGS) $(LFLAGS) $(INCLUDES) $(LIBS) -o $(TARGET) $(SOURCES) 

clean:
	rm $(TARGET)

