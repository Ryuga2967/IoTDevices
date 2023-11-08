CC=clang
CFLAGS=-O3 -pipe
SRC_FILES=$(wildcard ./*.c)
OBJ_FILES=$(patsubst ./%.c,./%.o,$(SRC_FILES))
OUT_FILE=pibroker
HEADERS=../broker.h
DBG=lldb

$(OUT_FILE): $(OBJ_FILES)
	$(CC) $(LDFLAGS) $(OBJ_FILES) -o $(OUT_FILE)

./%.o: ./%.c $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@

run: $(OUT_FILE)
	make -C plugins
	./$(OUT_FILE) plugins/*.so

clean:
	rm *.o $(OUT_FILE)
