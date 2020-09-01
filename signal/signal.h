#ifndef  __aspawn_signal_signal_H__
# define __aspawn_signal_signal_H__

# define _POSIX_C_SOURCE 201107L
# include <signal.h>

void pure_sigemptyset(sigset_t *set);
void pure_sigfillset(sigset_t *set);

/**
 * @return 0 on success, (-errno) on failure.
 */
int sig_blockall(sigset_t *oldset);

/**
 * @return 0 on success, (-errno) on failure.
 */
int sig_setmask(const sigset_t *set);

#endif
