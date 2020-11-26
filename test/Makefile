CC = clang

CFLAGS := -std=c11 -Wall -g -fsanitize=address
LDFLAGS := -fuse-ld=lld -Wl,--as-needed

## Objects to build
SRCS := $(shell find -name '*.c' ! -name 'utility.c')
OUTS := $(SRCS:.c=.out)

## Build rules
test: $(OUTS)
	./run_tests.sh "$(OUTS)"

utility.o: utility.c utility.h
	$(CC) $(CFLAGS) $< -c -o $@

../libaspawn.a:
	$(MAKE) -C ../

%.out: %.c ../aspawn.h ../syscall/syscall.h ../libaspawn.a Makefile utility.o
	$(CC) $(CFLAGS) $(LDFLAGS) ../libaspawn.a $< ../libaspawn.a utility.o ../libaspawn.a -o $@

clean:
	rm -f $(OUTS) utility.o

.PHONY: test clean

## Dependencies
test_syscall.out: ../syscall/clone3.h
