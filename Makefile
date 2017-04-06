CC = gcc
CFLAGS = -ansi -pedantic -g -Wall -Wextra -DDEBUG=1
LIBRARIES = whisper.o imagine.o

all: test

test: test.out

clean:
	rm *.o *.out

test.out: test.c $(LIBRARIES)
	$(CC) $(CFLAGS) $^ -o $@

whisper.o: whisper/*
	$(CC) -c $(CFLAGS) ./whisper/*.c -o whisper.o

imagine.o: imagine/*
	$(CC) -c $(CFLAGS) ./imagine/*.c -o imagine.o
