#!/usr/bin/env bash
cd $(dirname "$0")
gcc -ansi -pedantic -Wall -Wextra -O3 src/test.c -o test.out -I./src -pthread
