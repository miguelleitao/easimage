
# Makefile for easimage examples

CFLAGS=-I../src -g -Wall -Wextra
LDFLAGS=-L ../src 
LDLIBS=-lm -leasimage
EXAMPLES=blur easimgview camview sdlimgview

VERSION:=$(shell git describe --tags --long)

default: ${EXAMPLES}

all: ${EXAMPLES} 

run: blur
	LD_LIBRARY_PATH=../src ./blur

memtest: blur
	LD_LIBRARY_PATH=../src valgrind --leak-check=full --show-leak-kinds=all -v ./blur

sdlimgview: sdlimgview.c
	gcc -I/usr/include/SDL -DVERSION=${VERSION} $^ -o $@ -lSDL

easimgview: easimgview.o
	gcc -Wall -o $@ $< -l easimage

camview: camview.o
	gcc -Wall -o $@ $< -l easimage

%.o: %.c 
	gcc -Wall -O2 -c -DVERSION=${VERSION} -o $@ $<

clean:
	rm -rf ${EXAMPLES} *.o
