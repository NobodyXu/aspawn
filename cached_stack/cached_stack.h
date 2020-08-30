#ifndef  __aspawn_cached_stack_cached_stack_H__
# define __aspawn_cached_stack_cached_stack_H__

# include <stddef.h>

void init_cached_stack_internal(struct stack_t *cached_stack);

/**
 * @return 0 on success, (-errno) on failure.
 */
int cleanup_cached_stack_internal(const struct stack_t *cached_stack);

/**
 * First, align size to meet mmap and clone's requirement.
 * Then, (re)allocate the stack to `size` if the current one isn't large enough..
 *
 * @return 0 on success, (-errno) on failure.
 */
int allocate_stack(struct stack_t *cached_stack, size_t size);

#endif
