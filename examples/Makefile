
CFLAGS=-I../src -g -Wall -Wextra
LDFLAGS=-L ../src 
LDLIBS=-lm -leasimage
EXAMPLES=blur

all: ${EXAMPLES}

run: blur
	LD_LIBRARY_PATH=../src ./blur

memtest: blur
	LD_LIBRARY_PATH=../src valgrind --leak-check=full -v ./blur
