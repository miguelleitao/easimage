# Makefile for easimage

TARGET=easimage

${TARGET}: 
	$(MAKE) -C src

all: ${TARGET} examples

${TARGET}.md: README.md
	echo "\mainpage easimage" >$@
	cat $^ |grep -v \^.! |grep -v \^\#\ ${TARGET} >> $@

examples:
	$(MAKE) -C examples

clean:
	$(MAKE) -C src clean
	$(MAKE) -C examples clean

install: src/libeasimage.so
	cp $< /usr/lib/
	cp src/easimage.h /usr/include/


