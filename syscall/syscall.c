#include "syscall.h"

#include <sys/syscall.h>

long pure_syscall(long syscall_number, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6)
{
#ifdef __x86_64__
    __asm__  __volatile__ (
             "movq %%rdi, %%rax\n\t" /* Syscall number -> rax.  */
             "movq %%rsi, %%rdi\n\t" /* shift arg1 - arg5 */
             "movq %%rdx, %%rsi\n\t"
             "movq %%rcx, %%rdx\n\t"
             "movq %%r8, %%r10\n\t"
             "movq %%r9, %%r8\n\t"
             "movq 8(%%rsp), %%r9\n\t" /* arg6 is on the stack */
             "syscall\n\t"
             "ret"
        :
        :
        : "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9");
#endif

    __builtin_unreachable();
}

int psys_openat_impl(int dirfd, const char *pathname, int flags, mode_t mode)
{
    return pure_syscall2(SYS_openat, dirfd, (long) pathname, flags, mode);
}
int psys_close(int fd)
{
    return pure_syscall2(SYS_close, fd);
}
int psys_dup3(int oldfd, int newfd, int flags)
{
    return pure_syscall2(SYS_dup3, oldfd, newfd, flags);
}

int psys_chdir(const char *path)
{
    return pure_syscall2(SYS_chdir, (long) path);
}
int psys_fchdir(int fd)
{
    return pure_syscall2(SYS_fchdir, fd);
}

ssize_t psys_write(int fd, const void *buf, size_t count)
{
    return pure_syscall2(SYS_write, fd, (long) buf, count);
}

ssize_t psys_read(int fd, void *buf, size_t count)
{
    return pure_syscall2(SYS_read, fd, (long) buf, count);
}

int psys_execve(const char *pathname, char * const argv[], char * const envp[])
{
    return pure_syscall2(SYS_execve, (long) pathname, (long) argv, (long) envp);
}
