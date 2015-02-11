#!/bin/sh

cc -c -std=c89 -o build/obj/main.o src/main.c
cc -all_load -std=c89 -lgit2 -o build/prog build/obj/main.o
./build/prog test/repos/
