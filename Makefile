CC = clang
CXX = clang++

# Path
BUILD_DIR ?= .
## Install prefix
PREFIX ?= /usr/local/

# Prepare $(BUILD_DIR)
dirs=cached_stack create_pipe syscall signal clone_internal
$(shell mkdir -p $(addprefix $(BUILD_DIR)/, $(dirs)))

ifeq ($(DEBUG), true)
CFLAGS := -Og -g -Wall

ifeq ($(USE_SANITIZER), true)
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
OBJS := $(addprefix $(BUILD_DIR)/, $(SRCS:.c=.o))

TARGETS := $(BUILD_DIR)/libaspawn.so $(BUILD_DIR)/libaspawn.a

## Build rules
all: $(TARGETS)

.SECONDEXPANSION:

$(BUILD_DIR)/libaspawn.so: $(OBJS)
	$(CC) -std=c11 -fPIC $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/libaspawn.a: $(OBJS)
	llvm-ar rcsuT $@ $^

$(BUILD_DIR)/%.o: %.c $$(wildcard %.h)  Makefile
	$(CC) -std=c11 -fPIC -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: %.cc $$(wildcard %.h) $$(wildcard %.hpp) Makefile
	$(CXX) -std=c++17 -fPIC -c $(CXXFLAGS) $(CFLAGS) -o $@ $<

### Specialize rule
$(BUILD_DIR)/syscall/memory.o: syscall/memory.c syscall/syscall.h Makefile
	$(CC) -std=c11 -fPIC -c $(CFLAGS) -Ofast -fno-builtin -o $@ $<

clean:
	rm -f $(OBJS) $(TARGETS)

test: $(BUILD_DIR)/libaspawn.a
	$(MAKE) -C test

install: $(TARGETS) aspawn.h common.h syscall/syscall.h
	cp $(TARGETS) $(PREFIX)/lib/
	mkdir -p $(PREFIX)/include/aspawn/syscall
	cp aspawn.h common.h $(PREFIX)/include/aspawn/
	cp syscall/syscall.h $(PREFIX)/include/aspawn/syscall/
.PHONY: clean test install all

## Dependencies
aspawn.h: common.h

$(BUILD_DIR)/syscall/%.o: syscall/make_syscall.h
$(BUILD_DIR)/syscall/clone3.o: syscall/syscall.h
syscall/syscall.h: common.h

$(BUILD_DIR)/signal/signal.o: syscall/syscall.h

$(BUILD_DIR)/clone_internal/clone_internal.o: clone_internal/stack_growth.h aspawn.h syscall/clone3.h

$(BUILD_DIR)/cached_stack/cached_stack.o: aspawn.h clone_internal/stack_growth.h
