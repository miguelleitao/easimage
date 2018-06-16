# Makefile for easimage

TARGET=easimage

all: ${TARGET}

${TARGET}: 
	$(MAKE) -C src


${TARGET}.md: README.md
	echo "\mainpage easimage" >$@
	cat $^ |grep -v \^.! |grep -v \^\#\ ${TARGET} >> $@

clean:
	$(MAKE) -C src clean

install: src/libeasimage.so
	cp $< /usr/lib/
	cp src/easimage.h /usr/include/


