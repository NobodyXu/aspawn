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
PUBLIC int cleanup_stack(const struct Stack_t *cached_stack);

/**
 * Helper functions for reusing stacks
 */

/**
 * Forward declaration of Stacks.
 */
struct Stacks;

/**
 * @param stacks On success, *stacks will contain non-NULL value.
 * @return 0 on success, (-errno) on failure.
 *
 * This function is thread-safe.
 */
PUBLIC int init_stacks(struct Stacks **stacks, uint16_t max_stacks);

/**
 * @return If succeeds, *stack will contain the stack.
 *         If all stacks are currently occupied, NULL is stored into *stack.
 *
 * This function is thread-safe.
 */
PUBLIC struct Stack_t* get_stack(struct Stacks *stacks);

/**
 * @param fd must be return value of aspawn.
 * @return 0 on success, (-errno) on failure.
 *
 * This function is thread-safe.
 */
PUBLIC int add_stack_to_waitlist(const struct Stacks *stacks, const struct Stack_t *stack, int fd);

/**
 * @param readable_fds Upon success, a list of readable fds will be writen in it,
 *                     with .data.fd equals to the fd.
 * @return number of readable fds on success, (-errno) on failure.
 *
 * This function is thread-safe.
 */
PUBLIC int recycle_stack(struct Stacks *stacks, struct epoll_event readable_fds[], int max_nfd, int timeout);

# ifdef __cplusplus
}
# endif

#endif
