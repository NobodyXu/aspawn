# define _POSIX_C_SOURCE 201107L
# include <signal.h>

#include "signal.h"
#include "../syscall/syscall.h"

#include <stddef.h>
#include <string.h>
#include <errno.h>

int sig_blockall(void *oldset)
{
    sigset_t set;
    pure_sigfillset(&set);
    return psys_sigprocmask(SIG_BLOCK, &set, oldset);
}

int sig_setmask(const void *set)
{
    return psys_sigprocmask(SIG_SETMASK, set, NULL);
}
