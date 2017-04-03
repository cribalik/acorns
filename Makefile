CC = gcc
CFLAGS = -ansi -pedantic -g -Wall -Wextra -DDEBUG=1
LIBRARIES = whisper.o imagine.o

all: test.out test2.out

clean:
	rm *.o *.out

test.out: test.c whisper.o
	$(CC) $(CFLAGS) $^ -o $@

test2.out: test2.c whisper.o
	$(CC) $(CFLAGS) $^ -o $@

whisper.o: whisper/*
	$(CC) -c $(CFLAGS) ./whisper/*.c -o whisper.o

imagine.o: imagine/*
	$(CC) -c $(CFLAGS) ./imagine/*.c -o imagine.o
