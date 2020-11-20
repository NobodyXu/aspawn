#include "aspawn.h"

#include <stdlib.h>
#include <stdatomic.h>

#include <sys/epoll.h>
#include <unistd.h>

#include <err.h>
#include <errno.h>

struct Entry {
    struct Stack_t stack;
    size_t next;
};
struct Stacks {
    int epfd;
    uint16_t max_entries;

    _Atomic uint32_t free_list;
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
    p->entries[max_stacks - 1].next = max_stacks;

    *stacks = p;
    return 0;
}

#define GET_TIMESTAMP(val) ((val) & (UINT32_MAX - UINT16_MAX))
#define NEXT(val, old_val) ((val) | (GET_TIMESTAMP(old_val) + (UINT16_MAX + 1)))
#define GET_INDEX(val) ((val) & UINT16_MAX)

struct Stack_t* get_stack(struct Stacks *stacks)
{
    uint32_t free_list = atomic_load(&stacks->free_list);
    uint16_t index;
    do {
        index = GET_INDEX(free_list);
        if (index == stacks->max_entries)
            return NULL;
    } while (!atomic_compare_exchange_weak(&stacks->free_list, &free_list, 
                                           NEXT(stacks->entries[index].next, free_list)));

    return &stacks->entries[index].stack;
}

#define FD_BITS (sizeof(int) * 8)
#define FD_MASK ((int) -1)

int add_stack_to_waitlist(const struct Stacks *stacks, const struct Stack_t *stack, int fd)
{
    struct epoll_event event = {
        .events = EPOLLIN | EPOLLHUP | EPOLLET,
        .data = (epoll_data_t){
            .u64 = ( (((struct Entry*) stack) - stacks->entries) << FD_BITS) | fd
        }
    };
    if (epoll_ctl(stacks->epfd, EPOLL_CTL_ADD, fd, &event) < 0)
        return -errno;
    return 0;
}

int recycle_stack(struct Stacks *stacks, struct epoll_event readable_fds[], int max_nfd, int timeout)
{
    int nevent = epoll_wait(stacks->epfd, readable_fds, max_nfd, timeout);
    if (nevent < 0)
        return -errno;

    struct epoll_event dummy_event;

    int out = 0;
    for (int i = 0; i != nevent; ++i) {
        int fd = readable_fds[i].data.u64 & FD_MASK;
        size_t entry_index = readable_fds[i].data.u64 >> FD_BITS;

        if (readable_fds[i].events & EPOLLHUP) {
            uint32_t free_list = atomic_load(&stacks->free_list);
            uint16_t index;
            do {
                index = GET_INDEX(free_list);
                stacks->entries[entry_index].next = index;
            } while (!atomic_compare_exchange_weak(&stacks->free_list, &free_list, 
                                                   NEXT(entry_index, free_list)));

            if (readable_fds[i].events & EPOLLIN) {
                if (epoll_ctl(stacks->epfd, EPOLL_CTL_DEL, fd, &dummy_event) < 0)
                    err(1, "Bug: epoll_ctl in recycle_stack failed on %d", fd);
            } else {
                close(fd);
            }
        }
        if ((readable_fds[i].events & EPOLLIN)) {
            if (out != i)
                readable_fds[out] = readable_fds[i];
            readable_fds[out++].data.fd = fd;
        }
    }
    return out;
}
