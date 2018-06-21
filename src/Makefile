# Makefile to build and install easimage

VERSION:=$(shell git describe --tags --long)

-include .config

TARGET=libeasimage.so

all: easimage

easimage: ${TARGET}

install: ${TARGET}
	cp $< /usr/lib/
	cp easimage.h /usr/include/

libeasimage.so: camera.o image.o viewer.o util.o
	gcc -shared -Wall -O2 -Wl,-soname,$@,-z,defs -o $@ $^ -lSDL -lm

%.o: %.c easimage.h
	gcc -Wall -fPIC -O2 -c -o $@ $< 

clean:
	${RM} -- *.o *.so *.a ${TARGET}

