#define _GNU_SOURCE

#include <sys/syscall.h>
#include <sys/mman.h>

#include "../syscall/clone3.h"
#include "../syscall/clone.h"
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

#include <unistd.h>

int test_psys_clone3_fn(void *arg)
{
    assert(arg == NULL);
    ASSERT_SYSCALL(raise(SIGURG));

    return 0;
}
void test_psys_clone3_sa_handler(int signum)
{
    errx(1, "received signal SIGURG in child.");
}
void test_psys_clone3()
{
#ifdef SYS_clone3
    struct sigaction act;
    struct sigaction oldact;

    memset(&act, 0, sizeof(act));
    act.sa_handler = test_psys_clone3_sa_handler;

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

    long result = psys_clone3(&cl_args, sizeof(cl_args), test_psys_clone3_fn, NULL);
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

int test_psys_clone_fn(void *arg)
{
    assert(arg == NULL);
    return 0;
}
void test_psys_clone()
{
    int result = psys_clone(test_psys_clone_fn, NULL, SIGCHLD, NULL, /* optional args */ NULL, NULL, NULL);
    if (result < 0) {
        errno = -result;
        err(1, "%s on line %zu failed", "psys_clone", (size_t) __LINE__);
    }

    int wstatus;
    pid_t pid;
    ASSERT_SYSCALL((pid = wait(&wstatus)));

    assert(pid == result);
    assert(!WIFSIGNALED(wstatus));
    assert(WEXITSTATUS(wstatus) == 0);
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

        if (write(pipefd[1], "hello", sizeof("hello")) < 0)
            err(1, "write(%d) failed", pipefd[1]);
        char buffer[sizeof("hello")];
        if (read(pipefd[0], buffer, sizeof(buffer)) < 0)
            err(1, "read(%d) failed", pipefd[0]);
        assert(strcmp(buffer, "hello") == 0);
    }
}

/**
 * test psys_m*ap.
 */
void test_psys_maps()
{
    int errno_v;

    for (int i = 0; i != 10; ++i) {
        char *addr = psys_mmap(&errno_v, NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (addr == MAP_FAILED) {
            errno = errno_v;
            err(1, "psys_mmap failed");
        }
        for (size_t j = 0; j != 4096; ++j) {
            assert(addr[j] == 0);
            addr[j] = -1;
            assert(addr[j] == -1);
        }
        addr = psys_mremap(&errno_v, addr, 4096, 4096 * 2, MREMAP_MAYMOVE, NULL);
        if (addr == MAP_FAILED) {
            errno = errno_v;
            err(1, "psys_mremap failed");
        }
        for (size_t j = 0; j != 4096; ++j)
            assert(addr[j] == -1);
        for (size_t j = 4096; j != 4096 * 2; ++j) {
            assert(addr[j] == 0);
            addr[j] = -1;
            assert(addr[j] == -1);
        }
        errno_v = psys_munmap(addr, 4096 * 2);
        if (errno_v < 0) {
            errno = errno_v;
            err(1, "psys_munmap failed");
        }
    }
}

int main(int argc, char* argv[])
{
    test_psys_clone3();
    test_psys_clone();
    test_psys_pipe2();
    test_psys_maps();
}
