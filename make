#!/usr/bin/sh
gcc -o ./dst/$1 $1.c -lSDL2 && ./dst/$1
