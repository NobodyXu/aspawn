CC = clang
CXX = clang++

CFLAGS := -Ofast -Wall -flto
CFLAGS += -fno-asynchronous-unwind-tables -fno-unwind-tables -fmerge-all-constants -fuse-ld=lld

CXXFLAGS := -fno-exceptions -fno-rtti

LDFLAGS := -L /usr/local/lib/ -s -Wl,-icf=all,--gc-sections -flto -Wl,--plugin-opt=O3 -fuse-ld=lld
LIBS := -lbenchmark -lpthread

## Objects to build
C_SRCS   := $(wildcard *.c)
CXX_SRCS := $(wildcard *.cc)
OUTS := $(C_SRCS:.c=.out) $(CXX_SRCS:.cc=.out)

benchmark: pre-build $(OUTS)
	./run_benchmark.sh "$(OUTS)"

pre-build: 
	$(MAKE) -C ../

../libaspawn.so:
	$(MAKE) -C ../

%.out: %.c ../aspawn.h ../syscall/syscall.h ../libaspawn.so Makefile
	$(CC) -std=c11 $(CFLAGS) $(LDFLAGS) $(LIBS) $< ../libaspawn.so -o $@

%.out: %.cc ../aspawn.h ../syscall/syscall.h ../libaspawn.so Makefile
	$(CXX) -std=c++17 $(CFLAGS) $(CXXFLAGS) $(LDFLAGS) $(LIBS) $< ../libaspawn.so -o $@

clean:
	rm -f $(OUTS)

.PHONY: benchmark clean
