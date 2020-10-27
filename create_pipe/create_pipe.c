#define _GNU_SOURCE

#include "create_pipe.h"

#include <fcntl.h>
#include "../syscall/syscall.h"

int create_cloexec_pipe(int pipefd[2])
{
    return psys_pipe2(pipefd, O_CLOEXEC);
}
