#ifndef  __aspawn_syscall_syscall_H__
# define __aspawn_syscall_syscall_H__

# include <sys/types.h>
# include <fcntl.h>

# define GET_ARG_N(_null, _0, _1, _2, _3, _4, _5, _6, _7, ...) _7
# define GET_NARGS_(...) GET_ARG_N(__VA_ARGS__)
/**
 * GET_NARGS can at most detect nargs up util 7.
 */
# define GET_NARGS(...) GET_NARGS_(99, ## __VA_ARGS__, 7, 6, 5, 4, 3, 2, 1, 0)

/**
 * All psys_* here returns negative error code on failure and does not modify errno.
 *
 * The negative error code is equaivlent to (-errno).
 */

/**
 * Rationale on why syscall takes long:
 *  - https://stackoverflow.com/questions/35628927/what-is-the-type-of-system-call-arguments-on-linux
 */
long pure_syscall(long syscall_number, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6);

# define GET_syscall_args(_syscall_num, _1, _2, _3, _4, _5, _6, ...) _syscall_num, _1, _2, _3, _4, _5, _6
/**
 * pure_syscall2 takes at most 6 arguments.
 */
# define pure_syscall2(syscall_number, ...) \
    pure_syscall(GET_syscall_args(syscall_number, ## __VA_ARGS__, 0, 0, 0, 0, 0, 0))

int psys_openat_impl(int dirfd, const char *pathname, int flags, mode_t mode);
/**
 * @return in addition to errors specified in manpage for openat, 
 *         openat would return EINVAL when the presence of mode doesn't match the presence of 
 *         O_CREAT in flags.
 */
#define psys_openat(dirfd, pathname, flags, ... /* mode */) \
    ({                                           \
        int ret;                                 \
        if (GET_NARGS(__VA_ARGS__) == 0) {       \
            if ((flags & O_CREAT) != 0)          \
                ret = (-EINVAL);                 \
            else                                 \
                ret = psys_openat_impl((dirfd), (pathname), (flags), 0); \
        } else {                                 \
            if ((flags & O_CREAT) == 0)          \
                ret = (-EINVAL);                 \
            else                                 \
                ret = psys_openat_impl((dirfd), (pathname), (flags), __VA_ARGS__); \
        }                                        \
        ret;                                     \
     })

int psys_close(int fd);

int psys_dup3(int oldfd, int newfd, int flags);

int psys_chdir(const char *path);
int psys_fchdir(int fd);

ssize_t psys_write(int fd, const void *buf, size_t count);
ssize_t psys_read(int fd, void *buf, size_t count);

int psys_setresuid(uid_t ruid, uid_t euid, uid_t suid);
int psys_setresgid(gid_t rgid, gid_t egid, gid_t sgid);
int psys_setgroups(size_t size, const gid_t *list);

int psys_sched_setparam(pid_t pid, const void *param);
int psys_sched_getparam(pid_t pid, void *param);
int psys_sched_setscheduler(pid_t pid, int policy, const void *param);
int psys_sched_getscheduler(pid_t pid);

/**
 * param pid is removed from this function to improve portability to os other than linux.
 */
int psys_prlimit(int resource, const void *new_limit, void *old_limit);

/**
 * @return nice value in the range [40, 1], corresponding to commonly used [-20, 19].
 *         Translate between them using unice = 20 - knice.
 */
int psys_getpriority(int which, long who);
/**
 * @param knice in the range [40, 1], corresponding to commonly used [-20, 19].
 *              Translate between them using unice = 20 - knice.
 */
int psys_setpriority(int which, long who, int knice);

/**
 * @param ignore If ignore == 1, ignore the signal. Otherwise set it to default handler.
 */
int psys_sig_set_handler(int signum, int ignore);

/**
 * Check man sigprocmask for its API doc.
 */
int psys_sigprocmask(int how, const void *set, void *oldset);

int psys_execve(const char *pathname, char * const argv[], char * const envp[]);
/**
 * linux-specific call, checks `man 2 execveat` for more info.
 */
int psys_execveat(int dirfd, const char *pathname, char * const argv[], char * const envp[], int flags);

/**
 * @param file must not be NULL.
 * @param resolved_path resolved_path will only be changed on success.
 *                      Must be of path_max_len + 1.
 * @param PATH a pointer to the ':' separated string, containing directories to be looked up for the binary.
 *             Will be modified during call.
 *             On the first call, *PATH must point to the environment variable PATH.
 * @param path_max_len the max len of path (including the filename) on the system, 
 *                     excluding the trailing null byte.
 *
 * @return Returns 1 where next candidate path to exe is ready.
 *         Returns 0 where all possible path is tried.
 *
 * This is a copy of glibc's execvep's implementation, except that it doesn't use
 * optimized strchrnull or memcpy from glibc (due to ).
 * It simply concat each path with file and let the user tries it with execve :D
 * A smarter application can utilize a cache to speed this up for repeated execves of binaries.
 *
 * Usage:
 *     char resolved_path[PATH_MAX + 1];
 *     const char *path = // Get envir var path from parent;
 *     if (path == NULL || path[0] == '\0') {
 *         // *Handle that situation yourself*
 *     }
 *     if (argv[0] contains slash and is a path) {
 *         // In case where argv[0] is already a path, there is no need to call find_exe.
 *     }
 *     for (int got_eaccess = 0; find_exe(file, file_len, resolved_path, &path, PATH_MAX); ) {
 *         int result = handle_find_exe_err(psys_execve(resolved_path, argv, envp));
 *         if (result < 0) {
 *             int errno = -result;
 *             // executable is found, but it failed to execute
 *             // *Handle the errors here*
 *         }
 *     }
 *     // If got_eaccess, the the executable is probably found but not executable.
 *     // I.E., not a regular file, exe permission is denied, the filesystem is found is mounted noexec.
 *     //
 *     // But it could also be that the user do not have search permission on the prefix of resolved_path.
 */
int find_exe(const char *file, size_t file_len, char *resolved_path, 
                  const char **PATH, size_t path_max_len);

/**
 * @param result return value of execve or fexecve.
 * @return 0 to try next path in PATH, negative number for failure.
 *         In case negative number is returned, it is like psys_*: the negative number is equaivlent to
 *         (-errno).
 *
 * Check find_exe for doc.
 */
int handle_execve_err(int result, int *got_eaccess);

#endif
