// cd /path/to/aspawn/repo
// make -j $(nproc)
// sudo make install
// clang -std=c11 -L /usr/local/lib/ -laspawn example1.c
// ./a.out

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

#include <sys/wait.h>
#include <linux/limits.h>

static const char * const argv1[] = {"echo", "-e", "\nHello,", "world!", NULL};
static const char * const argv2[] = {"ls", NULL};
static const char * const argv3[] = {"uname", "-a", NULL};

static const char * const * argvs[] = {argv1, argv2, argv3};

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

static int fn(void *arg, int write_end_fd, void *old_sigset, void *user_data, size_t user_data_len)
{
    static const char * const envp[] = {NULL};

    const char * const * argv = arg;
    const char *path_env = user_data;
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
    struct Stack_t stack;
    init_cached_stack(&stack);

    char *path = getenv("PATH");
    if (path == NULL)
        errx(1, "PATH not found");
    if (path[0] == '\0')
        errx(1, "PATH is empty");
    const size_t path_sz = strlen(path) + 1;

    pid_t pids[3];
    int fds[3];
    for (size_t i = 0; i != sizeof(argvs) / sizeof(void*); ++i) {
        fds[i] = aspawn(&pids[i], &stack, PATH_MAX + 1, fn, (void*) argvs[i], path, path_sz);
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

    for (size_t i = 0; i != 3; ++i) {
        int wstatus;
        if (waitpid(pids[i], &wstatus, 0) < 0)
            err(1, "waitpid failed");

        if (WIFSIGNALED(wstatus))
            errx(1, "%ld is killed by signal!", (long) pids[i]);
        int exit_status = WEXITSTATUS(wstatus);
        if (exit_status != 0)
            errx(1, "%ld exited with %d", (long) pids[i], exit_status);

        // Check fds[i] for error message
        int result;
        for (; (result = splice(fds[i], NULL, 2, NULL, 20000, 0)) != 0; );
        if (result < 0)
            err(1, "splice failed");
    }

    int result = cleanup_stack(&stack);
    if (result < 0)
        errx(1, "cleanup_stack failed: %s", strerror(-result));

    return 0;
}
