CC = clang

CFLAGS := -std=c11 -Oz -fvisibility=hidden -Wall
CFLAGS += -fno-asynchronous-unwind-tables -fno-unwind-tables  -fmerge-all-constants

LDFLAGS = -s -shared -Wl,-soname,$@ -Wl,-icf=all,--gc-sections -flto -Wl,--plugin-opt=O3 -fuse-ld=lld

## Objects to build
SRCS := $(wildcard *.c) $(wildcard */*.c)
OBJS := $(SRCS:.c=.o)

## Build rules
aspawn: $(OBJS)
	$(CC) -fPIC $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

%.o: %.c %.h
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -f $(OBJS)
.PHONY: clean

## Dependencies
syscall/%.c: syscall/make_syscall.h
syscall/clone3.c: syscall/syscall.h
syscall/signal.c: signal/signal.h

signal/signal.c: syscall/syscall.h

clone_internal/clone_internal.c: clone_internal/stack_growth.h aspawn.h syscall/clone3.h

cached_stack/cached_stack.c: aspawn.h clone_internal/stack_growth.h
