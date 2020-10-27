#define _GNU_SOURCE

#include <sys/syscall.h>
#include "../syscall/clone3.h"
#include "../syscall/syscall.h"

#include "utility.h"

#include <stdio.h>
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

    struct psys_clone_args cl_args = {
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

void test_psys_pipe2()
{
    for (int i = 0; i != 10; ++i) {
        int pipefd[2];
        int ret = psys_pipe2(pipefd, O_CLOEXEC);
        if (ret < 0) {
            errno = -ret;
            err(1, "psys_pipe2 failed");
        }

        char path[sizeof("/proc/self/fdinfo/") + 20];
        for (int j = 0; j != 2; ++j) {
            int ret = snprintf(path, sizeof(path), "/proc/self/fdinfo/%d", pipefd[j]);
            if (ret < 0)
                err(1, "snprintf failed");
            if (ret >= sizeof(path))
                errx(1, "snprintf buffer not enough");

            FILE *f = fopen(path, "r");
            if (f == NULL)
                err(1, "fopen(%s, \"r\") failed", path);

            // pos:    0
            // flags:  02000001

            size_t pos;
            switch (fscanf(f, "pos: %zu", &pos)) {
                case EOF:
                    err(1, "fscanf failed");
                case 0:
                    errx(1, "fscanf is unable to parse %s of %s", "first line", path);
            }
            assert(pos == 0);

            size_t flags;
            switch (fscanf(f, "flags: %zu", &flags)) {
                case EOF:
                    err(1, "fscanf failed");
                case 0:
                    errx(1, "fscanf is unable to parse %s of %s", "second line", path);
            }
            assert(flags == 02000001L);

            fclose(f);
        }
    }
}

int main(int argc, char* argv[])
{
    test_clone();
    test_psys_pipe2();
}
