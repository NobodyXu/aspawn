CC = clang

CFLAGS := -std=c11 -Ofast -fvisibility=hidden -Wall -flto
CFLAGS += -fno-asynchronous-unwind-tables -fno-unwind-tables -fmerge-all-constants

LDFLAGS = -s -shared -Wl,-soname,$@ -Wl,-icf=all,--gc-sections -flto -Wl,--plugin-opt=O3 -fuse-ld=lld

## Objects to build
SRCS := $(shell find . -name '*.c' -a ! -wholename './test/*' -a ! -wholename './benchmark*')
OBJS := $(SRCS:.c=.o)

## Build rules
libaspawn.so: $(OBJS)
	$(CC) -fPIC $(LDFLAGS) -o $@ $^

libaspawn.a: $(OBJS)
	llvm-ar rcsuT $@ $^

%.o: %.c %.h Makefile
	$(CC) -fPIC -c $(CFLAGS) -o $@ $<

%.o: %.c Makefile
	$(CC) -fPIC -c $(CFLAGS) -o $@ $<

### Specialize rule
syscall/memory.o: syscall/memory.c syscall/syscall.h Makefile
	$(CC) -fPIC -c $(CFLAGS) -Ofast -fno-builtin -o $@ $<

clean:
	rm -f $(OBJS)
test: libaspawn.so
	$(MAKE) -C test
.PHONY: clean test

## Dependencies
aspawn.h: common.h

syscall/%.o: syscall/make_syscall.h
syscall/clone3.o: syscall/syscall.h
syscall/syscall.h: common.h

signal/signal.o: syscall/syscall.h

clone_internal/clone_internal.o: clone_internal/stack_growth.h aspawn.h syscall/clone3.h

cached_stack/cached_stack.o: aspawn.h clone_internal/stack_growth.h
