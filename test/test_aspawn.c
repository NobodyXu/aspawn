#include "../aspawn.h"
#include "../syscall/syscall.h"

#include "utility.h"

#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <poll.h>

#include <linux/limits.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <assert.h>

#include <err.h>
#include <errno.h>

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
#define psys_err(exit_status, s) \
    do {                         \
        psys_put(2, s "\n");   \
        return (exit_status);    \
    } while (0)

int psys_execvep(const char *file, size_t file_len, const char * const argv[], const char *path)
{
    static const char * const envp[] = {NULL};

    char constructed_path[PATH_MAX + 1];

    for (int got_eaccess = 0; ; ) {
        int result;
        switch (find_exe(file, file_len, constructed_path, &path, PATH_MAX)) {
            case 1:
                psys_put_impl(1, constructed_path, pstrlen(constructed_path));
                psys_put(1, "\n");
                result = handle_execve_err(psys_execve(constructed_path, argv, envp), &got_eaccess);
                if (result < 0)
                    psys_err(1, "Executable is found, but failed to execute it");
                continue;
    
            case -1:
                psys_err(1, "One of the environment path is longer than PATH_MAX");
    
            case 0:
                break;
        }
        if (got_eaccess)
            psys_err(1, "Executable is found but execve failed");
        psys_err(1, "Executable not found");
    }
}
int test_aspawn_fn(void *arg, int write_end_fd, void *old_sigset, void *user_data, size_t user_data_len)
{
    if (arg != NULL)
        psys_err(1, "arg != NULL");

    static const char * const argv[] = {"echo", "-e", "\nHello,", "world!", NULL};
    return psys_execvep(argv[0], 4, argv, user_data);
}
int main(int argc, char* argv[])
{
    struct Stack_t stack;

    init_cached_stack(&stack);

    char *path = getenv("PATH");
    const size_t path_sz = strlen(path) + 1;

    assert(path != NULL);
    assert(path[0] != '\0');

    pid_t pids[3];
    for (size_t i = 0; i != 3; ++i) {
        int result = aspawn(&pids[i], &stack, PATH_MAX + 1, test_aspawn_fn, NULL, path, path_sz);
        if (result < 0) {
            errno = -result;
            err(1, "aspawn failed");
        }

        struct pollfd pfd = {
            .fd = result,
            .events = POLLHUP,
        };
        if (poll(&pfd, 1, -1) < 0)
            err(1, "poll failed");

        close(result);

        int wstatus;
        ASSERT_SYSCALL((wait(&wstatus)));
        assert(!WIFSIGNALED(wstatus));
        assert(WEXITSTATUS(wstatus) == 0);
    }

    int result = cleanup_stack(&stack);
    if (result < 0) {
        errno = -result;
        err(1, "cleanup_stacks failed");
    }

    return 0;
}
