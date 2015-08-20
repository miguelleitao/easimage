
all: easimage

easimage: src/libeasimage.so

src/libeasimage.so:
	$(MAKE) -C src

install: src/libeasimage.so
	cp $< /usr/lib/
	cp src/easimage.h /usr/include/


