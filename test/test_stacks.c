#include "../aspawn.h"
#include "utility.h"

#include <unistd.h>

#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <linux/limits.h>

#include <assert.h>
#include <errno.h>
#include <err.h>

void check_for_recycling_stacks(struct Stacks *stacks)
{
    struct epoll_event fds[10];
    int errno_v = recycle_stack(stacks, fds, sizeof(fds) / sizeof(struct epoll_event), 0);
    if (errno_v < 0) {
        errno = -errno_v;
        err(1, "recycle_stack failed");
    }
}

int main(int argc, char* argv[])
{
    struct Stacks *stacks;

    int errno_v = init_stacks(&stacks, 200);
    if (errno_v != 0) {
        errno = errno_v;
        err(1, "init_stacks failed");
    }

    char *path = getenv("PATH");
    const size_t path_sz = strlen(path) + 1;

    assert(path != NULL);
    assert(path[0] != '\0');

    for (int i = 0; i != 400; ++i) {
        struct Stack_t *stack;

        do {
            check_for_recycling_stacks(stacks);
            stack = get_stack(stacks);
        } while (stack == NULL);

        pid_t pid;
        int fd = aspawn(&pid, stack, PATH_MAX + 1, test_aspawn_fn, NULL, path, path_sz);
        if (fd < 0) {
            errno = -errno_v;
            err(1, "aspawn failed");
        }

        errno_v = add_stack_to_waitlist(stacks, stack, fd);
        if (errno_v < 0) {
            errno = -errno_v;
            err(1, "add_stack_to_waitlist failed");
        }
    }

    errno_v = free_stacks(stacks);
    if (errno_v < 0) {
        errno = -errno_v;
        err(1, "free_stacks failed");
    }

    return 0;
}
