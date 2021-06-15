/* cd /path/to/aspawn/repo
 * make -j $(nproc)
 * sudo make install
 * cd example/
 * clang -std=c11 -L /usr/local/lib/ -laspawn example1.c
 * ./a.out
 *
 * If you prefer static link, then be sure to use `-fuse-ld=lld`
 */

#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include "../aspawn.h"
#include "../syscall/syscall.h"

#include <stdlib.h>
#include <string.h>
#include <err.h>

#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>

#include <sys/sendfile.h>
#include <sys/wait.h>
#include <linux/limits.h>

static const char * const argv1[] = {"echo", "-e", "\nHello,", "world!", NULL};
static const char * const argv2[] = {"ls", NULL};
static const char * const argv3[] = {"uname", "-a", NULL};

static const char * const * argvs[] = {argv1, argv2, argv3};

#define argv_cnt (sizeof(argvs) / sizeof(argvs[0]))

void psys_put_impl(int fd, const char *s, size_t len)
{
    size_t i = 0;
    do {
        ssize_t result = psys_write(fd, s + i, len - i);
        if (result < 0)
            return;
        i += result;
    } while (i != len);
}
#define psys_put(fd, s) psys_put_impl((fd), (s), sizeof(s))
#define psys_err(exit_status, fd, s) \
    do {                         \
        psys_put(fd, s "\n");   \
        return (exit_status);    \
    } while (0)

struct Args {
    const char * const * argv;
    char path[];
};

static int fn(void *arg, int write_end_fd, void *old_sigset)
{
    static const char * const envp[] = {NULL};

    struct Args *args = arg;

    const char * const * argv = args->argv;
    const char *path_env = args->path;
    size_t file_len = pstrlen(argv[0]);

    psys_sigprocmask(SIG_SETMASK, old_sigset, NULL);

    char constructed_path[PATH_MAX + 1];

    for (int got_eaccess = 0; ; ) {
        int result;
        switch (find_exe(argv[0], file_len, constructed_path, &path_env, PATH_MAX)) {
            case 1:
                result = handle_execve_err(psys_execve(constructed_path, argv, envp), &got_eaccess);
                if (result < 0)
                    psys_err(1, write_end_fd, "Executable is found, but failed to execute it");
                continue;
    
            case -1:
                psys_err(1, write_end_fd, "One of the environment path is longer than PATH_MAX");
    
            case 0:
                break;
        }
        if (got_eaccess)
            psys_err(1, write_end_fd, "Executable is found but execve failed");
        psys_err(1, write_end_fd, "Executable not found");
    }
}
int main(int argc, char* argv[])
{
    /* Get path */
    char *path = getenv("PATH");
    if (path == NULL)
        errx(1, "PATH not found");
    if (path[0] == '\0')
        errx(1, "PATH is empty");
    const size_t path_sz = strlen(path) + 1;

    struct Stack_t stack;
    init_cached_stack(&stack);

    pid_t pids[argv_cnt];
    int fds[argv_cnt];
    for (size_t i = 0; i != argv_cnt; ++i) {
        reserve_stack(&stack, PATH_MAX + 1, path_sz);

        struct Args *args = allocate_obj_on_stack(&stack, sizeof(struct Args) + path_sz);
        memcpy(args->path, path, path_sz);

        args->argv = argvs[i];

        fds[i] = aspawn(&pids[i], &stack, fn, args);
        if (fds[i] < 0)
            errx(1, "aspawn failed: %s", strerror(-fds[i]));

        // Wait for the child to execve or exit
        struct pollfd pfd = {
            .fd = fds[i],
            .events = POLLHUP,
        };
        if (poll(&pfd, 1, -1) < 0)
            err(1, "poll failed");
        // Now the stack is no longer used by the child process, we can reuse it
    }

    char buffer[4095];

    for (size_t i = 0; i != argv_cnt; ++i) {
        int wstatus;
        if (waitpid(pids[i], &wstatus, 0) < 0)
            err(1, "waitpid failed");

        if (WIFSIGNALED(wstatus))
            errx(1, "%ld is killed by signal!", (long) pids[i]);
        int exit_status = WEXITSTATUS(wstatus);
        if (exit_status != 0)
            errx(1, "%ld exited with %d", (long) pids[i], exit_status);

        // Check fds[i] for error message
        for (ssize_t result; (result = read(fds[i], buffer, sizeof(buffer))) != 0;) {
            if (result < 0)
                err(1, "read failed");
            do {
                int cnt = write(2, buffer, result);
                if (cnt < 0)
                    err(1, "write failed");
                result -= cnt;
            } while (result);
        }
    }

    int result = cleanup_stack(&stack);
    if (result < 0)
        errx(1, "cleanup_stack failed: %s", strerror(-result));

    return 0;
}
