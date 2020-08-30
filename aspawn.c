#include "signal/signal.h"

#include "aspawn.h"
#include "cached_stack/cached_stack.h"
#include "clone_internal/clone_internal.h"
#include "create_pipe/create_pipe.h"

#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include <sched.h>
#include <signal.h>

void init_cached_stack(struct stack_t *cached_stack)
{
    init_cached_stack_internal(cached_stack);
}

int cleanup_stacks(const struct stack_t *cached_stack)
{
    return cleanup_cached_stack_internal(cached_stack);
}

int aspawn_impl(pid_t *pid, struct stack_t *cached_stack, size_t additional_stack_requirement, 
                int (*fn)(void *arg), void *arg)
{
    int pipefd[2];
    int result = create_cloexec_pipe(pipefd);
    if (result < 0)
        return result;

    result = allocate_stack(cached_stack, (32 * 1024) + additional_stack_requirement);
    if (result < 0)
        return result;

    int new_pid = clone_internal(fn, arg, cached_stack);
    if (new_pid < 0)
        return new_pid;

    *pid = new_pid;

    close(pipefd[1]);

    return pipefd[0];
}
int aspawn(pid_t *pid, struct stack_t *cached_stack, size_t additional_stack_requirement, 
           int (*fn)(void *arg), void *arg)
{
    int oldstate;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);

    sigset_t oldset;
    int result = sig_blockall(&oldset);
    if (result < 0)
        goto fail_to_block_signal;

    result = aspawn_impl(pid, cached_stack, additional_stack_requirement, fn, arg);

    sig_setmask(&oldset);

fail_to_block_signal:
    pthread_setcancelstate(oldstate, NULL);

    return result;
}
