#include "../aspawn.h"
#include "../syscall/syscall.h"

#include "utility.h"

#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <poll.h>

#include <linux/limits.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <assert.h>

#include <err.h>
#include <errno.h>


int main(int argc, char* argv[])
{
    struct Stack_t stack;

    init_cached_stack(&stack);

    char *path = getenv("PATH");
    const size_t path_sz = strlen(path) + 1;

    assert(path != NULL);
    assert(path[0] != '\0');

    pid_t pids[3];
    for (size_t i = 0; i != 3; ++i) {
        int result = aspawn(&pids[i], &stack, PATH_MAX + 1, test_aspawn_fn, NULL, path, path_sz);
        if (result < 0) {
            errno = -result;
            err(1, "aspawn failed");
        }

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

    int result = cleanup_stack(&stack);
    if (result < 0) {
        errno = -result;
        err(1, "cleanup_stacks failed");
    }

    return 0;
}
