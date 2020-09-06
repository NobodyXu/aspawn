#ifndef  __aspawn_syscall_clone3_H__
# define __aspawn_syscall_clone3_H__

# include <stdint.h>
# include <stddef.h>

struct clone_args {
    uint64_t flags;        /* Flags bit mask */
    uint64_t pidfd;        /* Where to store PID file descriptor (pid_t *) */
    uint64_t child_tid;    /* Where to store child TID, in child's memory (pid_t *) */
    uint64_t parent_tid;   /* Where to store child TID, in parent's memory (int *) */
    uint64_t exit_signal;  /* Signal to deliver to parent on child termination */
    uint64_t stack;        /* Pointer to lowest byte of stack */
    uint64_t stack_size;   /* Size of stack */
    uint64_t tls;          /* Location of new TLS */
    uint64_t set_tid;      /* Pointer to a pid_t array */
    uint64_t set_tid_size; /* Number of elements in set_tid */
};

# define CLONE_CLEAR_SIGHAND 0x100000000ULL /* Clear any signal handler and reset to SIG_DFL. */

long psys_clone3(struct clone_args *cl_args, size_t size, int (*fn)(void *arg), void *arg);

#endif
