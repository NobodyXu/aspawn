#define _GNU_SOURCE

#include "create_pipe.h"

#include <fcntl.h>
#include <unistd.h>

#include <errno.h>

int create_cloexec_pipe(int pipefd[2])
{
    if (pipe2(pipefd, O_CLOEXEC) < 0)
        return -errno;
    return 0;
}
