CC = gcc
CFLAGS = -ansi -pedantic -g -Wall -Wextra

test: test.c whisper.o
	$(CC) $(CFLAGS) $^ -o $@

run: test
	test
	clean

%.o: %/%.c
	@echo $?
	@echo $@
	$(CC) -c $(CFLAGS) $^ -o $@
	

whisper.o: whisper/whisper.c
	@echo $?
	@echo $@
	$(CC) -c $(CFLAGS) $^ -o $@

