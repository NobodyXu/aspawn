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
#include <spawn.h>

#include <linux/limits.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <assert.h>

#include <err.h>
#include <errno.h>

#include <benchmark/benchmark.h>

static void wait_for_child()
{
    int wstatus;
    ASSERT_SYSCALL((wait(&wstatus)));
    assert(!WIFSIGNALED(wstatus));
    assert(WEXITSTATUS(wstatus) == 0);
}
static void Cleanup_stacks(const struct Stack_t *stack)
{
    int result = cleanup_stack(stack);
    if (result < 0) {
        errno = -result;
        err(1, "cleanup_stacks failed");
    }
}

static int bench_aspawn_fn(void *arg, int write_end_fd, void *old_sigset, void *user_data, size_t user_data_len)
{
    return 0;
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
        wait_for_child();
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
        wait_for_child();
        close(result);
        state.ResumeTiming();
    }
    cleanup_stack(&stack);
}
BENCHMARK(BM_aspawn)->Threads(1);

static void BM_posix_spawn(benchmark::State &state)
{
    pid_t pid;

    const char * const argv[] = {"echo", "-n", NULL};
    const char * const env[] = {NULL};

    posix_spawnattr_t attr;
    if (posix_spawnattr_init(&attr) != 0)
        err(1, "posix_spawnattr_init failed");
    if (posix_spawnattr_setflags(&attr, POSIX_SPAWN_USEVFORK) != 0)
        err(1, "posix_spawnattr_setflags failed");

    for ([[maybe_unused]] auto _: state) {
        int result = posix_spawnp(&pid, argv[0], NULL, &attr, (char *  const *) argv, (char * const *) env);
        if (result != 0) {
            errno = result;
            err(1, "posix_spawnp failed");
        }

        state.PauseTiming();
        wait_for_child();
        state.ResumeTiming();
    }

    posix_spawnattr_destroy(&attr);
}
BENCHMARK(BM_posix_spawn);

BENCHMARK_MAIN();
