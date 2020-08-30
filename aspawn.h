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
 * @param cached_stack on the first call to aspawn, cached_stack need to be created with init_cached_stack.
 * @param additional_stack_requirement should be the maximum size of variables that will be defined in
 *                                     the child.
 * 
 * @param fn If fn returns, then the child will call exit_group.
 *
 * @return fd of read end of pipe if success, eitherwise (-errno).
 */
int aspawn(pid_t *pid, struct stack_t *cached_stack, size_t additional_stack_requirement, 
           void (*fn)(void *arg), void *arg);

int cleanup_stacks(const struct stack_t *cached_stack);

#endif
