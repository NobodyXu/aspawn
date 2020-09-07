#include "signal.h"
#include "../syscall/syscall.h"

#include <stddef.h>
#include <string.h>
#include <errno.h>

int sig_blockall(sigset_t *oldset)
{
    sigset_t set;
    pure_sigfillset(&set);
    return psys_sigprocmask(SIG_BLOCK, &set, oldset);
}

int sig_setmask(const sigset_t *set)
{
    if (sigprocmask(SIG_SETMASK, set, NULL) < 0)
        return -errno;
    return 0;
}
