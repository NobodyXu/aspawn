#ifndef  __aspawn_cached_stack_cached_stack_H__
# define __aspawn_cached_stack_cached_stack_H__

# include "../common.h"
# include <stddef.h>

ALWAYS_INLINE void init_cached_stack_internal(struct Stack_t *cached_stack);

/**
 * @return 0 on success, (-errno) on failure.
 */
ALWAYS_INLINE int cleanup_cached_stack_internal(const struct Stack_t *cached_stack);

/**
 * First, align size to meet mmap and clone's requirement.
 * Then, (re)allocate the stack to `size` if the current one isn't large enough..
 *
 * @return 0 on success, (-errno) on failure.
 */
ALWAYS_INLINE int allocate_stack(struct Stack_t *cached_stack, size_t size, size_t obj_to_place_on_stack_len);

/**
 * @pre len <= stack->size
 */
ALWAYS_INLINE void* allocate_obj_on_stack(struct Stack_t *stack, size_t len);

#endif
