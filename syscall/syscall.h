#ifndef  __aspawn_syscall_syscall_H__
# define __aspawn_syscall_syscall_H__

/**
 * This function contains almost all functions that will be used in the aspawn-ed child.
 */

# include "../common.h"
# include <sys/types.h>
# include <fcntl.h>
# include <stddef.h>

# define GET_ARG_N(_null, _0, _1, _2, _3, _4, _5, _6, _7, ...) _7
# define GET_NARGS_(...) GET_ARG_N(__VA_ARGS__)
/**
 * GET_NARGS can at most detect nargs up util 7.
 */
# define GET_NARGS(...) GET_NARGS_(99, ## __VA_ARGS__, 7, 6, 5, 4, 3, 2, 1, 0)

# ifdef __cplusplus
extern "C" {
# endif

PUBLIC size_t pstrlen(const char *s);
/**
 * Copy exacting n bytes(excluding null byte) from src to dest.
 * Will append the null byte to dest.
 */
PUBLIC void pstrcpy(char *dest, const char *src, size_t n);

PUBLIC void pmemset(void *s, int c, size_t n);
PUBLIC void pmemcpy(void *dest, const void *src, size_t n);

/**
 * All psys_* here returns negative error code on failure and does not modify errno.
 *
 * The negative error code is equaivlent to (-errno).
 */

/**
 * Rationale on why syscall takes long:
 *  - https://stackoverflow.com/questions/35628927/what-is-the-type-of-system-call-arguments-on-linux
 */
PUBLIC long pure_syscall(long syscall_number, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6);

# define GET_syscall_args(_syscall_num, _1, _2, _3, _4, _5, _6, ...) _syscall_num, _1, _2, _3, _4, _5, _6
/**
 * pure_syscall2 takes at most 6 arguments.
 */
# define pure_syscall2(syscall_number, ...) \
    pure_syscall(GET_syscall_args(syscall_number, ## __VA_ARGS__, 0, 0, 0, 0, 0, 0))

/**
 * @param mode please set it to 0 when O_CREAT isn't specified in flags.
 */
PUBLIC int psys_openat(int dirfd, const char *pathname, int flags, mode_t mode);

PUBLIC int psys_close(int fd);

PUBLIC int psys_dup3(int oldfd, int newfd, int flags);

PUBLIC int psys_pipe2(int pipefd[2], int flag);

PUBLIC int psys_epoll_create1(int flags);
PUBLIC int psys_epoll_ctl(int epfd, int op, int fd, void *event);

PUBLIC int psys_chdir(const char *path);
PUBLIC int psys_fchdir(int fd);

PUBLIC ssize_t psys_write(int fd, const void *buf, size_t count);
PUBLIC ssize_t psys_read(int fd, void *buf, size_t count);

PUBLIC size_t psys_get_pagesz();

/**
 * @return MMAP_FAILED on error.
 */
PUBLIC void* psys_mmap(int *errno_v, void *addr, size_t len, int prot, int flags, int fd, off_t off);
PUBLIC int psys_munmap(void *addr, size_t len);

/**
 * @param new_addr please set it to NULL when MREMAP_FIXED isn't specified in flags.
 */
PUBLIC void* psys_mremap(int *errno_v, void *old_addr, size_t old_len, size_t new_len, int flags, 
                         void *new_addr);

PUBLIC int psys_setresuid(uid_t ruid, uid_t euid, uid_t suid);
PUBLIC int psys_setresgid(gid_t rgid, gid_t egid, gid_t sgid);
PUBLIC int psys_setgroups(size_t size, const gid_t *list);

PUBLIC int psys_sched_setparam(pid_t pid, const void *param);
PUBLIC int psys_sched_getparam(pid_t pid, void *param);
PUBLIC int psys_sched_setscheduler(pid_t pid, int policy, const void *param);
PUBLIC int psys_sched_getscheduler(pid_t pid);

/**
 * param pid is removed from this function to improve portability to os other than linux.
 */
PUBLIC int psys_prlimit(int resource, const void *new_limit, void *old_limit);

/**
 * @return nice value in the range [40, 1], corresponding to commonly used [-20, 19].
 *         Translate between them using unice = 20 - knice.
 */
PUBLIC int psys_getpriority(int which, long who);
/**
 * @param knice in the range [40, 1], corresponding to commonly used [-20, 19].
 *              Translate between them using unice = 20 - knice.
 */
PUBLIC int psys_setpriority(int which, long who, int knice);

ALWAYS_INLINE PUBLIC void pure_sigemptyset(void *set);
ALWAYS_INLINE PUBLIC void pure_sigfillset(void *set);

/**
 * Check man sigprocmask for its API doc.
 */
PUBLIC int psys_sigprocmask(int how, const void *set, void *oldset);

PUBLIC void psys_exit(int status);

PUBLIC int psys_execve(const char *pathname, const char * const argv[], const char * const envp[]);
/**
 * linux-specific call, checks `man 2 execveat` for more info.
 */
PUBLIC int psys_execveat(int dirfd, const char *pathname, char * const argv[], char * const envp[], int flags);

/**
 * @param file must not be NULL.
 * @param constructed_path constructed_path will only be changed on success.
 *                      Must be of path_max_len + 1.
 * @param PATH a pointer to the ':' separated string, containing directories to be looked up for the binary.
 *             Will be modified during call.
 *             On the first call, *PATH must point to the environment variable PATH.
 * @param path_max_len the max len of path (including the filename) on the system, 
 *                     excluding the trailing null byte.
 *
 * @return  1 when next candidate path to exe is ready;
 *         -1 when the constructed path will be longer than path_max_len;
 *          0 when all possible path is tried.
 *
 * This is a copy of glibc's execvep's implementation, except that it doesn't use
 * optimized strchrnull or memcpy from glibc (due to ).
 * It simply concat each path with file and let the user tries it with execve :D
 * A smarter application can utilize a cache to speed this up for repeated execves of binaries.
 *
 * Usage:
 *     char constructed_path[PATH_MAX + 1];
 *     const char *path = // Get envir var path from parent;
 *     if (path == NULL || path[0] == '\0') {
 *         // *Handle that situation yourself*
 *     }
 *     if (argv[0] contains slash and is a path) {
 *         // In case where argv[0] is already a path, there is no need to call find_exe.
 *     }
 *     for (int got_eaccess = 0; ; ) {
 *         int result;
 *         switch (find_exe(file, file_len, constructed_path, &path, PATH_MAX)) {
 *             case 1:
 *                 result = handle_execve_errexe_err(psys_execve(constructed_path, argv, envp), &got_eaccess);
 *                 if (result < 0) {
 *                     int errno_v = -result;
 *                     // executable is found, but it failed to execute
 *                     // *Handle the errors here*
 *                 }
 *                 continue;
 *
 *             case -1:
 *                 // One of the path is longer than path_max_len.
 *                 // Can chose the continue or break.
 *
 *             case 0:
 *                 break;
 *         }
 *         break;
 *     }
 *     // If got_eaccess, the the executable is probably found but not executable.
 *     // I.E., not a regular file, exe permission is denied, the filesystem is found is mounted noexec.
 *     //
 *     // But it could also be that the user do not have search permission on the prefix of constructed_path.
 */
PUBLIC int find_exe(const char *file, size_t file_len, char *constructed_path, 
                    const char **PATH, size_t path_max_len);

/**
 * @param result return value of execve or fexecve.
 * @return 0 to try next path in PATH, negative number for failure.
 *         In case negative number is returned, it is like psys_*: the negative number is equaivlent to
 *         (-errno).
 *
 * Check find_exe for doc.
 */
PUBLIC int handle_execve_err(int result, int *got_eaccess);

# ifdef __cplusplus
}
# endif

#endif
