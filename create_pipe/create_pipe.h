#ifndef  __aspawn_create_pipe_create_pipe_H__
# define __aspawn_create_pipe_create_pipe_H__

# include "../common.h"

/**
 * @return 0 on success, (-errno) on failure.
 */
ALWAYS_INLINE int create_cloexec_pipe(int pipefd[2]);

#endif
