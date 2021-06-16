# define _POSIX_C_SOURCE 201107L

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

#include <stdatomic.h>

struct aspawn_child_args {
    /* Improve locality of data */

    int pipefd[2];

    aspawn_fn fn;
    void *arg;

    /* These fields aren't used in aspawn_child, only taken address of */
    sigset_t old_sigset;
};

void init_cached_stack(struct Stack_t *cached_stack)
{
    init_cached_stack_internal(cached_stack);
}

int cleanup_stack(const struct Stack_t *cached_stack)
{
    return cleanup_cached_stack_internal(cached_stack);
}

int reserve_stack(
    struct Stack_t *cached_stack,
    size_t reserved_stack_sz, size_t obj_to_place_on_stack_len
)
{
    reserved_stack_sz += (32 * 1024);
    obj_to_place_on_stack_len += sizeof(struct aspawn_child_args);
    return allocate_stack(cached_stack, reserved_stack_sz, obj_to_place_on_stack_len);
}

int aspawn_child(void *arg);
int aspawn_child_clear_sighand(void *arg)
{
    /*
     * Clear all signal handler so that even user decided to restore masks,
     * the sig handler of the parent won't be accidentally called.
     */
    psys_sig_clearall_handler();

    return aspawn_child(arg);
}
int aspawn_child(void *arg)
{
    struct aspawn_child_args *args = arg;

    /*
     * This should always succeeds, unless there is bug in this library or the user of 
     * this library.
     */
    int ret = psys_close(args->pipefd[0]);
# ifndef NDEBUG
    if (ret < 0)
        perr(1, -ret, "psys_close in aspawn_child failed");
# endif

    return args->fn(args->arg, args->pipefd[1], &args->old_sigset);
}
int aspawn_impl(pid_t *pid, const struct Stack_t *cached_stack, aspawn_fn fn, void *arg, 
                const void *old_sigset)
{
    int pipefd[2];
    int result = create_cloexec_pipe(pipefd);
    if (result < 0)
        return result;

    struct Stack_t stack = *cached_stack;
    const size_t size = sizeof(struct aspawn_child_args);
    struct aspawn_child_args *args = allocate_obj_on_stack(&stack, size);

    args->pipefd[0] = pipefd[0];
    args->pipefd[1] = pipefd[1];

    args->fn = fn;
    args->arg = arg;
    pmemcpy(&args->old_sigset, old_sigset, sizeof(sigset_t));

    int new_pid;

    if (!HAS_CLONE_CLEAR_SIGHAND_INTERNAL) {
        new_pid = clone_internal(aspawn_child_clear_sighand, args, &stack);
    } else {
        /* Initialized to 0(false) */
        static atomic_bool does_not_have_clone_clear_sighand_internal;
        switch ((int) atomic_load(&does_not_have_clone_clear_sighand_internal)) {
            case 0:
                new_pid = clone_clear_sighand_internal(aspawn_child, args, &stack);
                if (new_pid != -ENOSYS && new_pid != -EINVAL)
                    /* The clone3 syscall exists but fail */
                    break;
                /* The clone3 syscall doesn't exist, now fallback to clone */
                atomic_store(&does_not_have_clone_clear_sighand_internal, 1);

            default:
            case 1:
                new_pid = clone_internal(aspawn_child_clear_sighand, args, &stack);
        }
    }

    if (new_pid >= 0) {
        *pid = new_pid;
        result = pipefd[0];
    } else {
        result = new_pid;
        psys_close(pipefd[0]);
    }

    psys_close(pipefd[1]);

    return result;
}
int aspawn(pid_t *pid, const struct Stack_t *cached_stack, aspawn_fn fn, void *arg)
{
    int oldstate;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);

    sigset_t oldset;
    int result = sig_blockall(&oldset);

    if (result == 0) {
        result = aspawn_impl(pid, cached_stack, fn, arg, &oldset);
        sig_setmask(&oldset);
    }

    pthread_setcancelstate(oldstate, NULL);

    return result;
}
int aspawn_rec(pid_t *pid, const struct Stack_t *cached_stack, aspawn_fn fn, void *arg, 
               const void *old_sigset)
{
    return aspawn_impl(pid, cached_stack, fn, arg, old_sigset);
}
