#include "aspawn.h"

#include <stdlib.h>
#include <stdatomic.h>

#include <sys/epoll.h>

#include <errno.h>

struct Entry {
    struct Stack_t stack;
    ptrdiff_t next;
};
struct Stacks {
    int epfd;
    uint16_t max_entries;

    _Atomic uint16_t free_list;
    struct Entry entries[];
};

int init_stacks(struct Stacks **stacks, uint16_t max_stacks)
{
    struct Stacks *p = calloc(1, sizeof(struct Stacks) + sizeof(struct Entry) * max_stacks);

    if (p == NULL)
        return -ENOMEM;

    p->epfd = epoll_create1(EPOLL_CLOEXEC);
    if (p->epfd == -1) {
        free(p);
        return -errno;
    }

    p->max_entries = max_stacks;
    for (uint16_t i = 0; i != max_stacks; ++i) {
        p->entries[i].next = i + 1;
    }
    p->entries[max_stacks - 1].next = -1;

    *stacks = p;
    return 0;
}

struct Stack_t* get_stack(struct Stacks *stacks)
{
    uint16_t free_list = atomic_load(&stacks->free_list);
    do {
        if (free_list == stacks->max_entries)
            return NULL;
    } while (!atomic_compare_exchange_weak(&stacks->free_list, &free_list, free_list + 1));

    return &stacks->entries[free_list].stack;
}
