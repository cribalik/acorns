CC = gcc
CFLAGS = -ansi -pedantic -g -Wall -Wextra

test.out: test.c whisper.o imagine.o
	$(CC) $(CFLAGS) $^ -o $@

run: test.out
	test.out

%.o:: %/* %/*
	$(CC) -c $(CFLAGS) ./$*/$*.c -o $@

