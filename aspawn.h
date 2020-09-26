#ifndef  __aspawn_aspawn_H__
# define __aspawn_aspawn_H__

/**
 * Make sure that function symbols in this project are immediately resolved if dynamically linked,
 * or make sure this library is statically linked.
 */

# include <stddef.h>
# include <sys/types.h>
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
 * @param old_sigset of type sigset_t*. The original value of sigmask.
 *                   It can be modified to any value user desired.
 *
 * The value of sigmask in aspawn_fn is unspecified.
 */
typedef int (*aspawn_fn)(void *arg, int wirte_end_fd, void *old_sigset, void *user_data, size_t user_data_len);

/**
 * @param pid the pid of the child will be stored into it on success.
 * @param cached_stack on the first call to aspawn, cached_stack need to be created with init_cached_stack.
 *                     Only modified on success.
 * @param reserved_stack_sz should be the maximum size of variables that will be defined in
 *                          the stack of the child.
 * @param fn If fn returns, then the child will exit with the return value of fn as the exit code.
 * @param user_data user data to be copied into stack.
 * @param user_data_len length of user_data
 *
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
PUBLIC int aspawn(pid_t *pid, struct Stack_t *cached_stack, size_t reserved_stack_sz, 
                  aspawn_fn fn, void *arg, void *user_data, size_t user_data_len);

/**
 * @param cached_stack must be 
 * @return 0 on success, (-errno) on failure.
 *
 * To reuse the destroyed stack, call init_cached_stack again.
 */
PUBLIC int cleanup_stacks(const struct Stack_t *cached_stack);

# ifdef __cplusplus
}
# endif

#endif
