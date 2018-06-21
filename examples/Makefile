
# Makefile for easimage examples

CFLAGS=-I../src -g -Wall -Wextra
LDFLAGS=-L ../src 
LDLIBS=-lm -leasimage
EXAMPLES=blur easimgview sdlimgview

default: blur

all: ${EXAMPLES} 

run: blur
	LD_LIBRARY_PATH=../src ./blur

memtest: blur
	LD_LIBRARY_PATH=../src valgrind --leak-check=full --show-leak-kinds=all -v ./blur

sdlimgview: sdlimgview.c
	gcc -I/usr/include/SDL $^ -o $@ -lSDL

clean:
	rm -rf ${EXAMPLES}
