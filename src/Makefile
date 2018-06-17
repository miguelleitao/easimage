# Makefile to build and install easimage

all: easimage

easimage: libeasimage.so

install: libeasimage.so
	cp $< /usr/lib/
	cp easimage.h /usr/include/

libeasimage.so: camera.o image.o viewer.o util.o
	gcc -shared -Wall -O2 -Wl,-soname,$@,-z,defs -o $@ $^ -lSDL -lm

%.o: %.c easimage.h
	gcc -Wall -fPIC -O2 -c -o $@ $< 

clean:
	rm -f *.o *.so *.a

