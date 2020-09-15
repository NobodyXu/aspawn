#define _GNU_SOURCE

#include "../syscall/clone3.h"
#include "../syscall/clone3.c"

#include "utility.h"

#include <stdint.h>
#include <errno.h>
#include <err.h>
#include <string.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/wait.h>

int test_clone_fn(void *arg)
{
    ASSERT_SYSCALL(raise(SIGURG));

    return 0;
}
void test_clone_sa_handler(int signum)
{
    errx(1, "received signal SIGURG in child.");
}
void test_clone()
{
#ifdef SYS_clone3
    struct sigaction act;
    struct sigaction oldact;

    memset(&act, 0, sizeof(act));
    act.sa_handler = test_clone_sa_handler;

    ASSERT_SYSCALL((sigaction(SIGURG, &act, &oldact)));

    struct clone_args cl_args = {
        .flags = CLONE_CLEAR_SIGHAND,

        .pidfd = (uint64_t) NULL,
        .child_tid = (uint64_t) NULL,
        .parent_tid = (uint64_t) NULL,

        .exit_signal = SIGCHLD,

        .stack = (uint64_t) NULL,
        .stack_size = 0,

        .tls = (uint64_t) NULL,
        .set_tid = (uint64_t) NULL,
        .set_tid_size = 0,
    };

    long result = psys_clone3(&cl_args, sizeof(cl_args), test_clone_fn, (void*) (uintptr_t) 10);
    if (result < 0) {
        if (-result == EINVAL) {
            warnx("In test_clone: You linux kernel does not support CLONE_CLEAR_SIGHAND");
            return;
        }
        errno = -result;
        err(1, "%s on line %zu failed", "psys_clone3", (size_t) __LINE__);
    }

    int wstatus;
    pid_t pid;
    ASSERT_SYSCALL((pid = wait(&wstatus)));

    assert(pid == result);
    assert(!WIFSIGNALED(wstatus));
    assert(WEXITSTATUS(wstatus) == 0);

    ASSERT_SYSCALL((sigaction(SIGURG, &oldact, NULL)));

#endif
}

int main(int argc, char* argv[])
{
    test_clone();
}
