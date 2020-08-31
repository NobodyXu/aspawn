#ifndef  __aspawn_aspawn_H__
# define __aspawn_aspawn_H__

# include <stddef.h>
# include <sys/types.h>

struct stack_t {
    void *addr;
    size_t size;
};

void init_cached_stack(struct stack_t *cached_stack);

/**
 * @param pid the pid of the child will be stored into it on success.
 * @param cached_stack on the first call to aspawn, cached_stack need to be created with init_cached_stack.
 *                     Only modified on success.
 * @param additional_stack_requirement should be the maximum size of variables that will be defined in
 *                                     the child.
 * @param fn If fn returns, then the child will call exit_group.
 * @return fd of read end of pipe if success, eitherwise (-errno).
 *
 * aspawn would disable thread cancellation, then it would revert it before return.
 *
 * Users of this call has to deal with signal handler accidently called in vm-shared child itself.
 */
int aspawn(pid_t *pid, struct stack_t *cached_stack, size_t additional_stack_requirement, 
           int (*fn)(void *arg), void *arg);

/**
 * @param cached_stack must be 
 * @return 0 on success, (-errno) on failure.
 */
int cleanup_stacks(const struct stack_t *cached_stack);

#endif
