#include "syscall.h"
#include "make_syscall.h"

#include <sys/syscall.h>
#include <errno.h>

#include <stddef.h>

long pure_syscall(long syscall_number, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6)
{
    return INTERNAL_SYSCALL(syscall_number, 6, arg1, arg2, arg3, arg4, arg5, arg6);
}

int psys_openat(int dirfd, const char *pathname, int flags, mode_t mode)
{
    return INTERNAL_SYSCALL(SYS_openat, 4, dirfd, pathname, flags, mode);
}
int psys_close(int fd)
{
    return INTERNAL_SYSCALL(SYS_close, 1, fd);
}
int psys_dup3(int oldfd, int newfd, int flags)
{
    return INTERNAL_SYSCALL(SYS_dup3, 3, oldfd, newfd, flags);
}

int psys_pipe2(int pipefd[2], int flag)
{
    return INTERNAL_SYSCALL(SYS_pipe2, 2, pipefd, flag);
}

int psys_epoll_create1(int flags)
{
    return INTERNAL_SYSCALL(SYS_epoll_create1, 1, flags);
}
int psys_epoll_ctl(int epfd, int op, int fd, void *event)
{
    return INTERNAL_SYSCALL(SYS_epoll_ctl, 4, epfd, op, fd, event);
}

int psys_chdir(const char *path)
{
    return INTERNAL_SYSCALL(SYS_chdir, 1, path);
}
int psys_fchdir(int fd)
{
    return INTERNAL_SYSCALL(SYS_fchdir, 1, fd);
}

ssize_t psys_write(int fd, const void *buf, size_t count)
{
    return INTERNAL_SYSCALL(SYS_write, 3, fd, buf, count);
}

ssize_t psys_read(int fd, void *buf, size_t count)
{
    return INTERNAL_SYSCALL(SYS_read, 3, fd, buf, count);
}

size_t psys_get_pagesz()
{
    return 4096;
}

#define MMAP_OFF_LOW_MASK  (psys_get_pagesz() - 1)

void *check_map_error(long result, int *errno_v)
{
    if (result > -4096UL) {
        *errno_v = -result;
        return (void*) -1;
    }
    return (void*) result;
}
void* psys_mmap(int *errno_v, void *addr, size_t len, int prot, int flags, int fd, off_t off)
{
    if (off & MMAP_OFF_LOW_MASK) {
        *errno_v = EINVAL;
        return (void*) -1;
    }

#ifdef SYS_mmap2
    return check_map_error(INTERNAL_SYSCALL(SYS_mmap2, 6, addr, len, prot, flags, fd, off / psys_get_pagesz()),
                           errno_v);
#else
    return check_map_error(INTERNAL_SYSCALL(SYS_mmap, 6, addr, len, prot, flags, fd, off), errno_v);
#endif
}

int psys_munmap(void *addr, size_t len)
{
    return INTERNAL_SYSCALL(SYS_munmap, 2, addr, len);
}

void* psys_mremap(int *errno_v, void *old_addr, size_t old_len, size_t new_len, int flags, void *new_addr)
{
    return check_map_error(INTERNAL_SYSCALL(SYS_mremap, 5, old_addr, old_len, new_len, flags, new_addr),
                           errno_v);
}

int psys_setresuid(uid_t ruid, uid_t euid, uid_t suid)
{
    return INTERNAL_SYSCALL(SYS_setresuid, 3, ruid, euid, suid);
}
int psys_setresgid(gid_t rgid, gid_t egid, gid_t sgid)
{
    return INTERNAL_SYSCALL(SYS_setresgid, 3, rgid, egid, sgid);
}
int psys_setgroups(size_t size, const gid_t *list)
{
    return INTERNAL_SYSCALL(SYS_setgroups, 2, size, list);
}

int psys_sched_setparam(pid_t pid, const void *param)
{
    return INTERNAL_SYSCALL(SYS_sched_setparam, 2, pid, param);
}
int psys_sched_getparam(pid_t pid, void *param)
{
    return INTERNAL_SYSCALL(SYS_sched_getparam, 2, pid, param);
}
int psys_sched_setscheduler(pid_t pid, int policy, const void *param)
{
    return INTERNAL_SYSCALL(SYS_sched_setscheduler, 3, pid, policy, param);
}
int psys_sched_getscheduler(pid_t pid)
{
    return INTERNAL_SYSCALL(SYS_sched_getscheduler, 1, pid);
}
int psys_getpriority(int which, long who)
{
    return INTERNAL_SYSCALL(SYS_getpriority, 2, which, who);
}
int psys_setpriority(int which, long who, int unice)
{
    return INTERNAL_SYSCALL(SYS_setpriority, 3, which, who, unice);
}

int psys_prlimit(int resource, const void *new_limit, void *old_limit)
{
    return INTERNAL_SYSCALL(SYS_prlimit64, 4, 0, resource, new_limit, old_limit);
}

void psys_exit(int status)
{
    INTERNAL_SYSCALL(SYS_exit, 1, status);
}

int psys_execve(const char *pathname, const char * const argv[], const char * const envp[])
{
    return INTERNAL_SYSCALL(SYS_execve, 3, pathname, argv, envp);
}
int psys_execveat(int dirfd, const char *pathname, char * const argv[], char * const envp[], int flags)
{
    return INTERNAL_SYSCALL(SYS_execveat, 5, dirfd, pathname, argv, envp, flags);
}

int find_exe(const char *file, size_t file_len, char *constructed_path, const char **PATH, size_t path_max_len)
{
    // Ignore empty path in *PATH
    for (; (*PATH)[0] == ':'; ++(*PATH));

    if ((*PATH)[0] == '\0')
        return 0;

    size_t i = 0;
    for (; (*PATH)[i] != '\0' && (*PATH)[i] != ':'; ++i) {
        if (i + 1 > path_max_len)
            return -1;
        constructed_path[i] = (*PATH)[i];
    }

    size_t path_prefix_len = i;

    if (constructed_path[i - 1] != '/')
        constructed_path[i++] = '/';

    if (i + file_len > path_max_len || /* Check for overflow */ i == 0 || i + file_len < i)
        return -1;

    pstrcpy(constructed_path + i, file, file_len);

    *PATH += path_prefix_len + ((*PATH)[path_prefix_len] == ':');

    return 1;
}

int handle_execve_err(int result, int *got_eaccess)
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
