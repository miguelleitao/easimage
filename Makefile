# Makefile for easimage

TARGET=easimage

PREFIX?=/usr/local

-include .config

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
	install -d $(DESTDIR)$(PREFIX)/lib/
	install -m 755 $< $(DESTDIR)$(PREFIX)/lib/
	install -d $(DESTDIR)$(PREFIX)/include/
	install -m 644 src/easimage.h $(DESTDIR)$(PREFIX)/include/


