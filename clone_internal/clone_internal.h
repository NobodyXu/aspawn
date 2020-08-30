#ifndef  __aspawn_clone_internal_clone_internal_H__
# define __aspawn_clone_internal_clone_internal_H__

/**
 * @return 0 on success, (-errno) on failure.
 */
int clone_internal(int (*fn)(void *arg), void *arg, struct stack_t *cached_stack);

#endif
