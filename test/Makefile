CC = clang

CFLAGS := -std=c11 -Wall -g -fsanitize=address
LDFLAGS := -fuse-ld=lld -Wl,--as-needed

## Objects to build
SRCS := $(wildcard *.c)
OUTS := $(SRCS:.c=.out)

## Build rules
test: $(OUTS)
	./run_tests.sh "$(OUTS)"

%.out: %.c ../aspawn.h ../syscall/syscall.h ../libaspawn.a Makefile
	$(MAKE) -C ../ libaspawn.a
	$(CC) $(CFLAGS) $(LDFLAGS) $< ../libaspawn.a -o $@

clean:
	rm -f $(OUTS)

.PHONY: test clean

## Dependencies
test_syscall.out: ../syscall/clone3.h
