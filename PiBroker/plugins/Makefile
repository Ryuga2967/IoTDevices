CC=clang
CFLAGS=-fPIC -rdynamic -shared
SRC_FILES=$(wildcard ./*.c)
OBJ_FILES=$(patsubst ./%.c,./%.so,$(SRC_FILES))
HEADERS=../broker.h
DBG=lldb

all: $(OBJ_FILES)

./%.so: ./%.c $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.so
