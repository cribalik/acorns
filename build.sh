#!/usr/bin/env bash
cd $(dirname "$0")
gcc -ansi -pedantic -Wall -Wextra -Wno-unused-function -g -DDEBUG=1 src/test.c -o test.out -I./src -pthread
