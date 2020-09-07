#ifndef  __aspawn_signal_signal_H__
# define __aspawn_signal_signal_H__

/**
 * @return 0 on success, (-errno) on failure.
 */
int sig_blockall(void *oldset);

/**
 * @return 0 on success, (-errno) on failure.
 */
int sig_setmask(const void *set);

#endif
