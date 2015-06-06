all: easimage cam_test

easimage: libeasimage.so

install: libeasimage.so
	#chmod 755 imgproc.py
	#cp imgproc.py /usr/local/lib/python2.7/dist-packages
	cp $< /usr/lib/
	cp easimage.h /usr/include/
	


libeasimage.so: camera.o image.o viewer.o util.o
	gcc -shared -Wall -O2 -Wl,-soname,$@,-z,defs -o $@ camera.o image.o viewer.o util.o -lSDL 
#-lSDLmain

%.o: %.c easimage.h
	gcc -std=c99 -Wall -fPIC -O2 -c -o $@ $< 

#-g -ggdb
#-I/usr/include/python2.7/

cam_test: cam_test.c
	cc -Wall -O2 -o cam_test cam_test.c -lm libeasimage.so

