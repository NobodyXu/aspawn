#ifndef  __aspawn_syscall_clone3_H__
# define __aspawn_syscall_clone3_H__

# include "../common.h"
# include <stddef.h>
# include <linux/types.h>

struct psys_clone_args {
    __aligned_u64 flags;        /* Flags bit mask */
    __aligned_u64 pidfd;        /* Where to store PID file descriptor (pid_t *) */
    __aligned_u64 child_tid;    /* Where to store child TID, in child's memory (pid_t *) */
    __aligned_u64 parent_tid;   /* Where to store child TID, in parent's memory (int *) */
    __aligned_u64 exit_signal;  /* Signal to deliver to parent on child termination */
    __aligned_u64 stack;        /* Pointer to lowest byte of stack */
    __aligned_u64 stack_size;   /* Size of stack */
    __aligned_u64 tls;          /* Location of new TLS */
    __aligned_u64 set_tid;      /* Pointer to a pid_t array */
    __aligned_u64 set_tid_size; /* Number of elements in set_tid */
};

# define CLONE_CLEAR_SIGHAND 0x100000000ULL /* Clear any signal handler and reset to SIG_DFL. */

ALWAYS_INLINE long psys_clone3(struct psys_clone_args *cl_args, size_t size, int (*fn)(void *arg), void *arg);

#endif
