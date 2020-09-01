#define _GNU_SOURCE

#include "../aspawn.h"
#include "clone_internal.h"
#include "stack_growth.h"

#include <sched.h>
#include <signal.h>

#include <errno.h>

#define STACK(addr, len) ((addr) + (len) * STACK_GROWS_DOWN)

int clone_internal(int (*fn)(void *arg), void *arg, const struct stack_t *cached_stack)
{
    int new_pid = clone(fn, STACK(cached_stack->addr, cached_stack->size), CLONE_VM | SIGCHLD, arg);
    if (new_pid == -1)
        return (-errno);
    return new_pid;
}
