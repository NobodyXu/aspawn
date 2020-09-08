#ifndef  __aspawn_clone_internal_clone_internal_H__
# define __aspawn_clone_internal_clone_internal_H__

# include "../common.h"
# include <sys/syscall.h>

/**
 * @return 0 on success, (-errno) on failure.
 */
ALWAYS_INLINE int clone_internal(int (*fn)(void *arg), void *arg, const struct stack_t *stack);

/**
 * HAS_CLONE_CLEAR_SIGHAND_INTERNAL is a compile time constant.
 *
 * You still need to test for -ENOSYS that can be returned from clone_clear_sighand_internal.
 */
# ifdef   SYS_clone3
#  define HAS_CLONE_CLEAR_SIGHAND_INTERNAL 1
# else
#  define HAS_CLONE_CLEAR_SIGHAND_INTERNAL 0
# endif

/**
 * @return -ENOSYS if syscall clone3 is not supported clone3.
 *         -EINVAL might mean that CLONE_CLEAR_SIGHAND is not supported, or stack is not aligned,
 *         according to https://elixir.bootlin.com/linux/latest/source/kernel/fork.c.
 */
ALWAYS_INLINE int clone_clear_sighand_internal(int (*fn)(void *arg), void *arg, const struct stack_t *stack);

#endif
