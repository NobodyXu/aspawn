#include "signal.h"
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
