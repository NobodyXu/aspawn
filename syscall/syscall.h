#ifndef  __aspawn_syscall_syscall_H__
# define __aspawn_syscall_syscall_H__

/**
 * Rationale on why syscall takes long:
 *  - https://stackoverflow.com/questions/35628927/what-is-the-type-of-system-call-arguments-on-linux
 */
long pure_syscall(long syscall_number, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6);

int psys_close(int fd);

#endif
