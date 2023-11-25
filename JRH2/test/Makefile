CC=clang
CFLAGS=-g3 -pipe -I..
DEPTENT_LIB=..
DEPTENT_FILES=$(wildcard ../*.c)
DEPENT_OBJS=$(DEPTENT_FILES:%.c=%.o)
SRC_FILES=$(wildcard ./*.c)
TARGETS=$(SRC_FILES:%.c=%.o)
DBG=lldb

define submake
	make -C${DEPTENT_LIB} $1
endef

all: ./main

../%.o: %.c %.h
	@$(call submake,$<)

./%.o: %.c %.h

./main: ${TARGETS} ${DEPENT_OBJS}
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean

clean:
	rm -rf ./main ./*.o
	@$(call submake,$@)
