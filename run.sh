#!/bin/sh

prgname=gitud
testrepospath=test/repos

cc -c -std=c89 -o build/obj/main.o src/main.c
cc -all_load -std=c89 -lgit2 -o build/$prgname build/obj/main.o
./build/$prgname $testrepospath
