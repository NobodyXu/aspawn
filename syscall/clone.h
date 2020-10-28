#ifndef  __aspawn_syscall_clone_H__
# define __aspawn_syscall_clone_H__

# include "../common.h"
# include <sys/types.h>

ALWAYS_INLINE int psys_clone(int (*fn)(void *arg), void *stack, int flags, void *arg,
                             /* optional args */ pid_t *parent_tid, void *tls, pid_t *child_tid);

#endif
