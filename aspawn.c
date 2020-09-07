#include "signal/signal.h"

#include "aspawn.h"
#include "cached_stack/cached_stack.h"
#include "clone_internal/clone_internal.h"
#include "create_pipe/create_pipe.h"
#include "syscall/syscall.h"

#include <string.h>

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

struct aspawn_child_args {
    /* Improve locality of data */

    int pipefd[2];

    aspawn_fn fn;
    void *arg;
    size_t user_data_len;

    /* These fields aren't used in aspawn_child, only taken address of */
    sigset_t old_sigset;
    char user_data[];
};

int aspawn_child(void *arg);
int aspawn_child_clear_sighand(void *arg)
{
    for (int sig = 1; sig < _NSIG; ++sig)
        psys_sig_set_handler(sig, 0);

    return aspawn_child(arg);
}
int aspawn_child(void *arg)
{
    struct aspawn_child_args *args = arg;

    psys_close(args->pipefd[0]);

    return args->fn(args->arg, args->pipefd[1], &args->old_sigset, args->user_data, args->user_data_len);
}
int aspawn_impl(pid_t *pid, struct stack_t *cached_stack, size_t reserved_stack_sz, 
                aspawn_fn fn, void *arg, void *user_data, size_t user_data_len, const void *old_sigset)
{
    int pipefd[2];
    int result = create_cloexec_pipe(pipefd);
    if (result < 0)
        return result;

    size_t objs_on_stack_len = sizeof(struct aspawn_child_args) + user_data_len;
    result = allocate_stack(cached_stack, (32 * 1024) + reserved_stack_sz, objs_on_stack_len);
    if (result < 0)
        goto fail;

    struct stack_t stack = *cached_stack;
    struct aspawn_child_args *args = allocate_obj_on_stack(&stack, objs_on_stack_len);

    args->pipefd[0] = pipefd[0];
    args->pipefd[1] = pipefd[1];

    args->fn = fn;
    args->arg = arg;
    args->user_data_len = user_data_len;

    memcpy(&args->old_sigset, old_sigset, sizeof(sigset_t));
    memcpy(args->user_data, user_data, user_data_len);

    int new_pid;

    switch (HAS_CLONE_CLEAR_SIGHAND_INTERNAL) {
        case 1:
            new_pid = clone_clear_sighand_internal(aspawn_child, args, &stack);
            if (new_pid != -ENOSYS)
                break;

        case 0:
            new_pid = clone_internal(aspawn_child_clear_sighand, args, &stack);
    }

    if (new_pid >= 0) {
        *pid = new_pid;
        result = pipefd[0];
    } else {
        result = new_pid;
fail:
        close(pipefd[0]);
    }

    close(pipefd[1]);

    return result;
}
int aspawn(pid_t *pid, struct stack_t *cached_stack, size_t reserved_stack_sz, 
           aspawn_fn fn, void *arg, void *user_data, size_t user_data_len)
{
    int oldstate;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);

    sigset_t oldset;
    int result = sig_blockall(&oldset);
    if (result < 0)
        goto fail_to_block_signal;

    result = aspawn_impl(pid, cached_stack, reserved_stack_sz, fn, arg, user_data, user_data_len, &oldset);

    sig_setmask(&oldset);

fail_to_block_signal:
    pthread_setcancelstate(oldstate, NULL);

    return result;
}
