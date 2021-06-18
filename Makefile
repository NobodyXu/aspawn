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

.DEFAULT_GOAL := all

## Automatic dependency building
DEPFLAGS = -MT $@ -MMD -MP -MF $(BUILD_DIR)/$*.Td
DEPFILES := $(OBJS:%.o=%.d)

## Dummy target for $(DEPFILES) when they are not present.
$(DEPFILES):
## Use wildcard so that nonexisitent dep files are ignored.
include $(wildcard $(DEPFILES))

## Build rules
.SECONDEXPANSION:

all: $(TARGETS)

$(BUILD_DIR)/libaspawn.so: $(OBJS)
	$(CC) -std=c11 -fPIC $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/libaspawn.a: $(OBJS)
	llvm-ar rcsuT $@ $^

$(BUILD_DIR)/%.o: %.c $$(wildcard %.h)  Makefile
	$(CC) -std=c11 -fPIC -c $(CFLAGS) $(DEPFLAGS) -o $@ $<
	mv -f $(BUILD_DIR)/$*.Td $(BUILD_DIR)/$*.d && touch $@

$(BUILD_DIR)/%.o: %.cc $$(wildcard %.h) $$(wildcard %.hpp) Makefile
	$(CXX) -std=c++17 -fPIC -c $(CXXFLAGS) $(CFLAGS) $(DEPFLAGS) -o $@ $<
	mv -f $(BUILD_DIR)/$*.Td $(BUILD_DIR)/$*.d && touch $@

### Specialize rule
$(BUILD_DIR)/syscall/memory.o: syscall/memory.c syscall/syscall.h Makefile
	$(CC) -std=c11 -fPIC -c $(CFLAGS) $(DEPFLAGS) -Ofast -fno-builtin -o $@ $<

clean:
	rm -f $(OBJS) $(TARGETS) $(DEPFILES)

test: $(BUILD_DIR)/libaspawn.a
	$(MAKE) -C test

install: $(TARGETS) aspawn.h common.h syscall/syscall.h
	cp $(TARGETS) $(PREFIX)/lib/
	mkdir -p $(PREFIX)/include/aspawn/syscall
	cp aspawn.h common.h $(PREFIX)/include/aspawn/
	cp syscall/syscall.h $(PREFIX)/include/aspawn/syscall/
.PHONY: clean test install all
