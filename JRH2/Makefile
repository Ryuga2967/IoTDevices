CC=clang
CFLAGS=-O3
SRC_FILES=$(wildcard ./*.c)
OBJ_FILES=$(SRC_FILES:%.c=%.o)
DBG=lldb

all: ${OBJ_FILES}

%.o: %.c %.h

.PHONY: clean

clean:
	rm -rf *.o
