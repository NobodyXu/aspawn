#define _GNU_SOURCE

#include "../aspawn.h"
#include "cached_stack.h"
#include "../clone_internal/stack_growth.h"

#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

void init_cached_stack_internal(struct stack_t *cached_stack)
{
    cached_stack->addr = NULL;
    cached_stack->size = 0;
}

int cleanup_cached_stack_internal(const struct stack_t *cached_stack)
{
    if (cached_stack->addr != NULL && cached_stack->size != 0) {
        int result = munmap(cached_stack->addr, cached_stack->size);
        if (result == -1)
            return -errno;
    }

    return 0;
}

size_t align(size_t sz, size_t alignment)
{
    size_t remnant = sz % alignment;
    return sz + remnant != 0 ? (alignment - remnant) : 0;
}
size_t align_to_page(size_t sz)
{
    return align(sz, sysconf(_SC_PAGESIZE));
}
size_t align_stack_sz(size_t sz)
{
    sz = align_to_page(sz);
#ifdef __aarch64__
    sz = align(align(sz, 15), 16);
#endif
    return sz;
}

int allocate_stack(struct stack_t *cached_stack, size_t size, size_t obj_to_place_on_stack_len)
{
    const int prot = PROT_READ | PROT_WRITE;
    const int flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK;

    size_t stack_size = align_to_page(align_stack_sz(size) + obj_to_place_on_stack_len);

    void *stack;

    /**
     * TODO: Allocate a 'null' page that is inaccessible at the end of the stack to help preventing
     * stack overflow error from overwriting other memory region.
     */

    if (cached_stack->addr != NULL) {
        if (cached_stack->size < stack_size)
            stack = mremap(cached_stack->addr, cached_stack->size, stack_size, MREMAP_MAYMOVE);
        else
            return 0;
    } else
        stack = mmap(NULL, stack_size, prot, flags, -1, 0);

    if (stack == MAP_FAILED)
        return -errno;
    cached_stack->addr = stack;
    cached_stack->size = stack_size;

    return 0;
}

void* allocate_obj_on_stack(struct stack_t *stack, size_t len)
{
    void *ret;

    stack->size -= len;
    if (STACK_GROWS_DOWN) {
        ret = stack->addr + stack->size;
    } else {
        ret = stack->addr;
        stack->addr += len;
    }
    
    return ret;
}
