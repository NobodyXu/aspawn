/**
 * bench_aspawn_throughput.cc benchmarks how fast can aspawn creates a new
 * process with and without reusing stack when there is no additional stack usage.
 */

#include "../aspawn.h"
#include "../syscall/syscall.h"

#include "../test/utility.h"

#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <pthread.h>
#include <poll.h>
#include <spawn.h>

#include <linux/limits.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <assert.h>

#include <err.h>
#include <errno.h>

#include <benchmark/benchmark.h>

static void Cleanup_stacks(const struct Stack_t *stack)
{
    int result = cleanup_stack(stack);
    if (result < 0) {
        errno = -result;
        err(1, "cleanup_stacks failed");
    }
}

static const char * const argv[] = {"/bin/true", NULL};
static const char * const envp[] = {NULL};

static void waitfor_stack_reuse(int fd)
{
    struct pollfd pfd = {
        .fd = fd,
        .events = POLLHUP,
    };
    if (poll(&pfd, 1, -1) < 0)
        err(1, "poll failed");
}
static int bench_aspawn_fn(void *arg, int write_end_fd, void *old_sigset, void *user_data, size_t user_data_len)
{
    psys_sigprocmask(SIG_SETMASK, old_sigset, NULL);
    psys_execve(argv[0], argv, envp);
    return 1;
}
static void BM_aspawn_no_reuse(benchmark::State &state)
{
    for ([[maybe_unused]] auto _: state) {
        struct Stack_t stack;

        pid_t pid;

        init_cached_stack(&stack);
        int result = aspawn(&pid, &stack, 0, bench_aspawn_fn, NULL, NULL, 0);
        if (result < 0) {
            errno = -result;
            err(1, "aspawn failed");
        }

        state.PauseTiming();

        waitfor_stack_reuse(result);
        close(result);
        Cleanup_stacks(&stack);

        state.ResumeTiming();
    }
}
BENCHMARK(BM_aspawn_no_reuse);

static void BM_aspawn(benchmark::State &state)
{
    struct Stack_t stack;
    init_cached_stack(&stack);

    for ([[maybe_unused]] auto _: state) {
        pid_t pid;

        int result = aspawn(&pid, &stack, 0, bench_aspawn_fn, NULL, NULL, 0);
        if (result < 0) {
            errno = -result;
            err(1, "aspawn failed");
        }

        state.PauseTiming();
        waitfor_stack_reuse(result);
        close(result);
        state.ResumeTiming();
    }
    cleanup_stack(&stack);
}
BENCHMARK(BM_aspawn)->Threads(1);

static void sig_blockall(sigset_t *oldset)
{
    sigset_t set;
    sigfillset(&set);
    sigprocmask(SIG_SETMASK, &set, oldset);
}
static void BM_vfork_with_shared_stack(benchmark::State &state)
{
    int oldstate;
    for ([[maybe_unused]] auto _: state) {
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);

        sigset_t oldset;
        sig_blockall(&oldset);

        pid_t pid = vfork();
        if (pid < 0)
            err(1, "vfork failed");
        if (pid == 0) {
            psys_sigprocmask(SIG_SETMASK, &oldset, NULL);
            psys_execve(argv[0], argv, envp);
            _exit(1);
        }

        sigprocmask(SIG_SETMASK, &oldset, NULL);
        pthread_setcancelstate(oldstate, NULL);
    }
}
BENCHMARK(BM_vfork_with_shared_stack);

static void BM_fork(benchmark::State &state)
{
    for ([[maybe_unused]] auto _: state) {
        pid_t pid = fork();
        if (pid < 0)
            err(1, "vfork failed");
        if (pid == 0) {
            psys_execve(argv[0], argv, envp);
            _exit(1);
        }
    }
}
BENCHMARK(BM_fork);

static void BM_posix_spawn(benchmark::State &state)
{
    pid_t pid;

    posix_spawnattr_t attr;
    if (posix_spawnattr_init(&attr) != 0)
        err(1, "posix_spawnattr_init failed");
    if (posix_spawnattr_setflags(&attr, POSIX_SPAWN_USEVFORK) != 0)
        err(1, "posix_spawnattr_setflags failed");

    for ([[maybe_unused]] auto _: state) {
        int result = posix_spawnp(&pid, argv[0], NULL, &attr, (char *  const *) argv, (char * const *) envp);
        if (result != 0) {
            errno = result;
            err(1, "posix_spawnp failed");
        }
    }

    posix_spawnattr_destroy(&attr);
}
BENCHMARK(BM_posix_spawn);

static void* wait_thread(void*)
{
    int wstatus;
    pid_t pid;
    int exit_status;
    for (; ;) {
        pid = wait(&wstatus);
        if (pid == -1 && errno == ECHILD)
            continue;
        if (WIFSIGNALED(wstatus))
            errx(1, "pid %ld is terminated by signal", (long) pid);
        exit_status = WEXITSTATUS(wstatus);
        if (exit_status != 0)
            errx(1, "pid %ld exited with %d", (long) pid, exit_status);
    }

    return NULL;
}

int main(int argc, char** argv) {
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv))
        return 1;

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, wait_thread, NULL);

    ::benchmark::RunSpecifiedBenchmarks();
    return 0;
}
