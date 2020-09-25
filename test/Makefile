CC = clang

CFLAGS := -std=c11 -Wall -g #-fsanitize=address
LDFLAGS := -fuse-ld=lld -Wl,--as-needed

## Objects to build
SRCS := $(wildcard *.c)
OUTS := $(SRCS:.c=.out)

## Build rules
test: $(OUTS)
	./run_tests.sh "$(OUTS)"

%.out: %.c ../aspawn.h ../syscall/syscall.h ../libaspawn.so Makefile
	$(MAKE) -C ../
	$(CC) $(CFLAGS) $(LDFLAGS) $< ../libaspawn.so -o $@

clean:
	rm -f $(OUTS)

.PHONY: test clean

## Dependencies
test_syscall.o: ../syscall/clone3.c
