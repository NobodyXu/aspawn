CC = clang
CXX = clang++

ifeq ($(DEBUG), true)
CFLAGS := -Og -g -Wall

ifneq ($(NO_SANITIZER), true)
CFLAGS += -fsanitize=address
endif

else
CFLAGS := -Ofast -fvisibility=hidden -Wall -flto
CFLAGS += -fno-asynchronous-unwind-tables -fno-unwind-tables -fmerge-all-constants
endif

CXXFLAGS := -fno-exceptions -fno-rtti

LDFLAGS = -s -shared -Wl,-soname,$@ -Wl,-icf=all,--gc-sections -flto -Wl,--plugin-opt=O3 -fuse-ld=lld

## Objects to build
SRCS := $(shell find . -name '*.c' -a ! -wholename './test/*' -a ! -wholename './benchmark/*' -a ! -wholename './example/*')
OBJS := $(SRCS:.c=.o)

## Install prefix
PREFIX := /usr/local/

## Build rules
all: libaspawn.so libaspawn.a

libaspawn.so: $(OBJS)
	$(CC) -std=c11 -fPIC $(LDFLAGS) -o $@ $^

libaspawn.a: $(OBJS)
	llvm-ar rcsuT $@ $^

%.o: %.c %.h Makefile
	$(CC) -std=c11 -fPIC -c $(CFLAGS) -o $@ $<

%.o: %.c Makefile
	$(CC) -std=c11 -fPIC -c $(CFLAGS) -o $@ $<

%.o: %.cc Makefile
	$(CXX) -std=c++17 -fPIC -c $(CXXFLAGS) $(CFLAGS) -o $@ $<

### Specialize rule
syscall/memory.o: syscall/memory.c syscall/syscall.h Makefile
	$(CC) -std=c11 -fPIC -c $(CFLAGS) -Ofast -fno-builtin -o $@ $<

clean:
	rm -f $(OBJS) libaspawn.so libaspawn.a
test: libaspawn.a
	$(MAKE) -C test
install: libaspawn.so libaspawn.a aspawn.h common.h syscall/syscall.h
	cp libaspawn.* $(PREFIX)/lib/
	mkdir -p $(PREFIX)/include/aspawn/syscall
	cp aspawn.h common.h $(PREFIX)/include/aspawn/
	cp syscall/syscall.h $(PREFIX)/include/aspawn/syscall/
.PHONY: clean test install all

## Dependencies
aspawn.h: common.h

syscall/%.o: syscall/make_syscall.h
syscall/clone3.o: syscall/syscall.h
syscall/syscall.h: common.h

signal/signal.o: syscall/syscall.h

clone_internal/clone_internal.o: clone_internal/stack_growth.h aspawn.h syscall/clone3.h

cached_stack/cached_stack.o: aspawn.h clone_internal/stack_growth.h
