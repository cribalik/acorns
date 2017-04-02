CC = gcc
CFLAGS = -ansi -pedantic -g -Wall -Wextra -DDEBUG
LIBRARIES = whisper.o imagine.o

all: test.out test2.out

%.out: %.c $(LIBRARIES)
	$(CC) $(CFLAGS) $^ -o $@

%.o:: %/*
	$(CC) -c $(CFLAGS) ./$*/$*.c -o $@
