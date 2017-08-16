#!/usr/bin/env bash
cd $(dirname "$0")
gcc -ansi -pedantic -Wall -Wextra -g -DDEBUG=1 src/test.c -o test.out -I./src -pthread
