#ifndef  __aspawn_signal_signal_H__
# define __aspawn_signal_signal_H__

# include "../common.h"

ALWAYS_INLINE int psys_sig_clear_handler(int signum);

/**
 * @return 0 on success, (-errno) on failure.
 */
ALWAYS_INLINE int sig_blockall(void *oldset);

/**
 * @return 0 on success, (-errno) on failure.
 */
ALWAYS_INLINE int sig_setmask(const void *set);

#endif
