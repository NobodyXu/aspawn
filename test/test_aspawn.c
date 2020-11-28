#include "../aspawn.h"
#include "../syscall/syscall.h"

#include "utility.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <poll.h>

#include <linux/limits.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <assert.h>
#include <err.h>

int main(int argc, char* argv[])
{
    struct Stack_t stack;

    init_cached_stack(&stack);

    char *path = getenv("PATH");
    const size_t path_sz = strlen(path) + 1;

    assert(path != NULL);
    assert(path[0] != '\0');

    printf("$PATH = %s\nsizeof PATH = %zu\n", path, path_sz);
    fflush(stdout);

    pid_t pids[3];
    for (size_t i = 0; i != 3; ++i) {
        int result = ASSERT_ASPAWNF(aspawn(&pids[i], &stack, PATH_MAX + 1, test_aspawn_fn, path, path, path_sz));

        struct pollfd pfd = {
            .fd = result,
            .events = POLLHUP,
        };
        if (poll(&pfd, 1, -1) < 0)
            err(1, "poll failed");

        close(result);

        int wstatus;
        ASSERT_SYSCALL((wait(&wstatus)));
        assert(!WIFSIGNALED(wstatus));
        assert(WEXITSTATUS(wstatus) == 0);
    }

    ASSERT_ASPAWNF(cleanup_stack(&stack));

    return 0;
}
