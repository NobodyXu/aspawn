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
    ASSERT_ASPAWNF(recycle_stack(stacks, fds, sizeof(fds) / sizeof(struct epoll_event), 0));
}

int main(int argc, char* argv[])
{
    struct Stacks *stacks;

    ASSERT_ASPAWNF(init_stacks(&stacks, 200));

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
        int fd = ASSERT_ASPAWNF(aspawn(&pid, stack, PATH_MAX + 1, test_aspawn_fn, NULL, path, path_sz));

        ASSERT_ASPAWNF(add_stack_to_waitlist(stacks, stack, fd));
    }

    ASSERT_ASPAWNF(free_stacks(stacks));

    return 0;
}
