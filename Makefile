all: easimage

easimage: libeasimage.so

install: libeasimage.so
	cp $< /usr/lib/
	cp easimage.h /usr/include/
	

libeasimage.so: camera.o image.o viewer.o util.o
	gcc -shared -Wall -O2 -Wl,-soname,$@,-z,defs -o $@ camera.o image.o viewer.o util.o -lSDL 
#-lSDLmain

%.o: %.c easimage.h
	gcc -std=c99 -Wall -fPIC -O2 -c -o $@ $< 

push:
	git commit
	git push  https://github.com/miguelleitao/easimage.git master


