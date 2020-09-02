#include "syscall.h"
#include <sys/syscall.h>
#include <errno.h>

#include <stddef.h>

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

int psys_setresuid(uid_t ruid, uid_t euid, uid_t suid)
{
    return pure_syscall2(SYS_setresuid, ruid, euid, suid);
}
int psys_setresgid(gid_t rgid, gid_t egid, gid_t sgid)
{
    return pure_syscall2(SYS_setresgid, rgid, egid, sgid);
}

int psys_execve(const char *pathname, char * const argv[], char * const envp[])
{
    return pure_syscall2(SYS_execve, (long) pathname, (long) argv, (long) envp);
}
int psys_execveat(int dirfd, const char *pathname, char * const argv[], char * const envp[], int flags)
{
    return pure_syscall2(SYS_execveat, dirfd, (long) pathname, (long) argv, (long) envp, flags);
}

int psys_find_exe(const char *file, size_t file_len, char *resolved_path, const char **PATH, size_t path_max_len)
{
    // Ignore empty path in *PATH
    for (; (*PATH)[0] == ':'; ++(*PATH));

    if ((*PATH)[0] == '\0')
        return 0;

    size_t i = 0;
    for (; (*PATH)[i] != '\0' && (*PATH)[i] != ':'; ++i) {
        if (i + 1 > path_max_len)
            return 0;
        resolved_path[i] = (*PATH)[i];
    }

    size_t path_prefix_len = i;

    if (resolved_path[i - 1] != '/')
        resolved_path[i++] = '/';

    if (i + file_len > path_max_len)
        return 0;

    for (; file[i] != '\0'; ++i)
        resolved_path[i] = file[i];

    *PATH += path_prefix_len + ((*PATH)[path_prefix_len] == ':');

    return 1;
}

int handle_find_exe_err(int result, int *got_eaccess)
{
    switch (-result) {
        case EACCES:
            // Record that we got a 'Permission denied' error.  If we end
            // up finding no executable we can use, we want to diagnose
            // that we did find one but were denied access.
            *got_eaccess = 1;
        case ENOENT:
        case ESTALE:
        case ENOTDIR:
            // Those errors indicate the file is missing or not executable
            // by us, in which case we want to just try the next path
            // directory.
        case ENODEV:
        case ETIMEDOUT:
            // Some strange filesystems like AFS return even
            // stranger error numbers.  They cannot reasonably mean
            // anything else so ignore those, too.
            return 0;
    
        default:
            // Some other error means we found an executable file, but
            // something went wrong executing it.
            return result;
    }
}
