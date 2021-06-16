#ifndef  __aspawn_aspawn_H__
# define __aspawn_aspawn_H__

/**
 * Make sure that function symbols in this project are immediately resolved if dynamically linked,
 * or make sure this library is statically linked.
 */

# include <stddef.h>
# include <stdint.h>
# include <sys/types.h>
# include <sys/epoll.h>
# include "common.h"

# ifdef __cplusplus
extern "C" {
# endif

struct Stack_t {
    void *addr;
    size_t size;
};

PUBLIC void init_cached_stack(struct Stack_t *cached_stack);

/**
 * @param reserved_stack_sz should be the maximum size of variables that will be
 *                          defined in the stack of the child.
 * @return 0 on success, (-errno) on failure.
 *
 * First, align size to meet mmap and clone's requirement
 * Then, (re)allocate the stack to `size` if the current one isn't large enough.
 */
PUBLIC int reserve_stack(
    struct Stack_t *cached_stack,
    size_t reserved_stack_sz, size_t obj_to_place_on_stack_len
);

/**
 * @pre You must have called reserved_stack(stack, ..., len)
 *
 * The way allocate_obj_on_stack allocates object is similar to how stack is used
 * during normal function execution.
 *
 * This is done to improve locality and avoid allocating an empty page just
 * for these objects.
 */
PUBLIC void* allocate_obj_on_stack(struct Stack_t *stack, size_t len);

/**
 * @param old_sigset of type sigset_t*. The original value of sigmask.
 *                   It can be modified to any value user desired.
 *
 * The value of sigmask in aspawn_fn is unspecified.
 */
typedef int (*aspawn_fn)(void *arg, int wirte_end_fd, void *old_sigset);

/**
 * @param pid the pid of the child will be stored into it on success.
 * @param cached_stack must call reserve_stack before using aspawn
 * @param fn If fn returns, then the child will exit with the return value of fn as the exit code.
 * @return fd of read end of CLOEXEC pipe if success, eitherwise (-errno).
 *
 * aspawn would disable thread cancellation, then it would revert it before return.
 *
 * aspawn would also mask all signals in parent and reset the signal handler in the child process.
 * Before aspawn returns in parent, it would revert the signal mask.
 *
 * In the function fn, you can only use syscall declared in syscall/syscall.h
 * Use of any glibc function or any function that modifies global/thread-local variable is undefined behavior.
 */
PUBLIC int aspawn(pid_t *pid, const struct Stack_t *cached_stack, aspawn_fn fn, void *arg);

/**
 * recursive version of aspawn that can be called inside aspawn_fn.
 *
 * @param old_sigset should be old_sigset from aspawn_fn.
 */
PUBLIC int aspawn_rec(pid_t *pid, const struct Stack_t *cached_stack,
                      aspawn_fn fn, void *arg, const void *old_sigset);

/**
 * @param cached_stack must be 
 * @return 0 on success, (-errno) on failure.
 *
 * To reuse the destroyed stack, call init_cached_stack again.
 */
PUBLIC int cleanup_stack(const struct Stack_t *cached_stack);

# ifdef __cplusplus
}
# endif

#endif
