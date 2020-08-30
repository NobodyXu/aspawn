#include "signal.h"

#include <stddef.h>
#include <errno.h>

int sig_blockall(sigset_t *oldset)
{
    sigset_t set;
    if (sigfillset(&set) < 0)
        return -errno;
    if (sigprocmask(SIG_BLOCK, &set, oldset) < 0)
        return -errno;
    return 0;
}

int sig_setmask(const sigset_t *set)
{
    if (sigprocmask(SIG_SETMASK, set, NULL) < 0)
        return -errno;
    return 0;
}
