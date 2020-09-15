CC = clang

CFLAGS := -std=c11 -Wall -fsanitize=address
LDFLAGS := -fuse-ld=lld -Wl,--as-needed

## Objects to build
SRCS := $(wildcard *.c)
OUTS := $(SRCS:.c=.out)

## Build rules
test: $(OUTS)

%.out: %.c ../aspawn.h ../syscall/syscall.h ../libaspawn.so Makefile
	$(CC) $(CFLAGS) $(LDFLAGS) $< ../libaspawn.so -o $@
	LD_LIBRARY_PATH=../ ./$@

clean:
	rm -f $(OUTS)

.PHONY: test clean

## Dependencies
test_syscall.o: ../syscall/clone3.c