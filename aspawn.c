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
    int pipefd[2];

    int (*fn)(void *arg, int wirte_end_fd, void *on_stack_obj, size_t len);
    void *arg;

    size_t user_data_len;
    char user_data[];
};
int aspawn_child(void *arg)
{
    struct aspawn_child_args *args = arg;

    psys_close(args->pipefd[0]);
    return args->fn(args->arg, args->pipefd[1], args->user_data, args->user_data_len);
}
int aspawn_impl(pid_t *pid, struct stack_t *cached_stack, size_t additional_stack_requirement, 
                aspawn_fn fn, void *arg, void *obj_to_place_on_stack, size_t len)
{
    int pipefd[2];
    int result = create_cloexec_pipe(pipefd);
    if (result < 0)
        return result;

    size_t objs_on_stack_len = sizeof(struct aspawn_child_args) + len;
    result = allocate_stack(cached_stack, (32 * 1024) + additional_stack_requirement, objs_on_stack_len);
    if (result < 0)
        return result;

    struct stack_t stack = *cached_stack;
    void *obj_addr = allocate_obj_on_stack(&stack, objs_on_stack_len);

    struct aspawn_child_args *args = obj_addr;
    args->pipefd[0] = pipefd[0];
    args->pipefd[1] = pipefd[1];
    args->fn = fn;
    args->arg = arg;

    args->user_data_len = len;
    memcpy(args->user_data, obj_to_place_on_stack, len);

    int new_pid = clone_internal(aspawn_child, args, &stack);
    if (new_pid < 0)
        return new_pid;

    *pid = new_pid;

    close(pipefd[1]);

    return pipefd[0];
}
int aspawn(pid_t *pid, struct stack_t *cached_stack, size_t additional_stack_requirement, 
           aspawn_fn fn, void *arg, void *obj_to_place_on_stack, size_t len)
{
    int oldstate;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);

    sigset_t oldset;
    int result = sig_blockall(&oldset);
    if (result < 0)
        goto fail_to_block_signal;

    result = aspawn_impl(pid, cached_stack, additional_stack_requirement, fn, arg, obj_to_place_on_stack, len);

    sig_setmask(&oldset);

fail_to_block_signal:
    pthread_setcancelstate(oldstate, NULL);

    return result;
}
