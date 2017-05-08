CC = gcc
CFLAGS = -ansi -pedantic -Wall -Wextra
DEBUGFLAGS = -g -DDEBUG=1
RELEASEFLAGS = -O3
CINCLUDE = -Iinclude
LIBRARIES = whisper.o imagine.o text.o
INCLUDES = include/*.h

all: debug

.PHONY: debug
debug: CFLAGS += $(DEBUGFLAGS)
debug: test.out

.PHONY: release
release: CFLAGS += $(RELEASEFLAGS)
release: test.out

.PHONY: clean
clean:
	rm -f *.o *.out

test.out: test/test.c $(LIBRARIES) $(INCLUDES)
	$(CC) $(CFLAGS) $(LIBRARIES) test/test.c -o $@ $(CINCLUDE)

text.o: text/*
	$(CC) -c $(CFLAGS) ./text/*.c -o text.o $(CINCLUDE)

whisper.o: whisper/*
	$(CC) -c $(CFLAGS) ./whisper/*.c -o whisper.o $(CINCLUDE)

imagine.o: imagine/*
	$(CC) -c $(CFLAGS) ./imagine/*.c -o imagine.o $(CINCLUDE)
