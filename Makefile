CC = gcc
CFLAGS = -ansi -pedantic -g -Wall -Wextra -DDEBUG=1
CINCLUDE = -Iinclude
LIBRARIES = whisper.o imagine.o text.o array.o

all: test

test: test.out

clean:
	rm -f *.o *.out

test.out: test/test.c $(LIBRARIES)
	$(CC) $(CFLAGS) $^ -o $@ $(CINCLUDE)

text.o: text/*
	$(CC) -c $(CFLAGS) ./text/*.c -o text.o $(CINCLUDE)

whisper.o: whisper/*
	$(CC) -c $(CFLAGS) ./whisper/*.c -o whisper.o $(CINCLUDE)

imagine.o: imagine/*
	$(CC) -c $(CFLAGS) ./imagine/*.c -o imagine.o $(CINCLUDE)

array.o: array/*
	$(CC) -c $(CFLAGS) ./array/*.c -o array.o $(CINCLUDE)
