#define _GNU_SOURCE

#include "../aspawn.h"

#include "clone_internal.h"
#include "stack_growth.h"

#include "../syscall/clone3.h"
#include "../syscall/clone.h"
#include <sys/syscall.h>

#include <stdint.h>

#include <sched.h>
#include <signal.h>

#include <errno.h>

#define STACK(addr, len) ((addr) + (len) * STACK_GROWS_DOWN)

int clone_internal(int (*fn)(void *arg), void *arg, const struct Stack_t *stack)
{
    return psys_clone(fn, STACK((char*) stack->addr, stack->size), CLONE_VM | SIGCHLD, arg,
                      /* optional args */ NULL, NULL, NULL);
}

int clone_clear_sighand_internal(int (*fn)(void *arg), void *arg, const struct Stack_t *stack)
{
#ifdef SYS_clone3
    struct psys_clone_args cl_args = {
        .flags = CLONE_VM | CLONE_CLEAR_SIGHAND,

        .pidfd = (uint64_t) NULL,
        .child_tid = (uint64_t) NULL,
        .parent_tid = (uint64_t) NULL,

        .exit_signal = SIGCHLD,

        .stack = (uint64_t) stack->addr,
        .stack_size = stack->size,

        .tls = (uint64_t) NULL,
        .set_tid = (uint64_t) NULL,
        .set_tid_size = 0,
    };

    return psys_clone3(&cl_args, sizeof(cl_args), fn, arg);
#else
    return -ENOSYS;
#endif
}
