
CFLAGS=-I../src
LDFLAGS=-L ../src -lm -leasimage 
EXAMPLES=blur

all: ${EXAMPLES}

run: blur
	LD_LIBRARY_PATH=../src ./blur
