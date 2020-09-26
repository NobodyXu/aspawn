# aspawn

Asynchronous and more versatile replacement of posix_spawn

## Intro

`aspawn` implements the idea from [`avfork`][4], enabling user to do `posix_spawn` in an asynchronous way by
making syscall directly via `pure_syscall` implemented in my library that does not use
any global/thread local variable at all.

My `aspawn` has signature:

```{c}
struct Stack_t {
    void *addr;
    size_t size;
};

typedef int (*aspawn_fn)(void *arg, int wirte_end_fd, void *old_sigset, void *user_data, size_t user_data_len);

/**
 * @return fd of read end of CLOEXEC pipe if success, eitherwise (-errno).
 *
 * aspawn would disable thread cancellation, then it would revert it before return.
 *
 * aspawn would also mask all signals in parent and reset the signal handler in the child process.
 * Before aspawn returns in parent, it would revert the signal mask.
 *
 * In the function fn, you can only use syscall declared in syscall/syscall.h
 * Use of any glibc function or any function that modifies global/thread-local variable is undefined behavior.
 */
int aspawn(pid_t *pid, struct Stack_t *cached_stack, size_t reserved_stack_sz, 
           aspawn_fn fn, void *arg, void *user_data, size_t user_data_len);
```

By returning the write end of the `CLOEXEC` pipefd, user of this library is able to receive error message/check whether
the child process has done using `cached_stack` so that `aspawn` can reuse `cached_stack`.

It also allows user to pass arbitary data in the stack via `user_data` and `user_data_len`, which get copies onto top of
the stack, thus user does not have to allocate them separately on heap or mistakenly overwriten an object used in child process.

To use a syscall, you need to include [`syscall/syscall.h`][2], which defines the syscall routine used by the child process including
`find_exe`, `psys_execve` and `psys_execveat`.

Compare to [`posix_spawn`][3], `aspawn` has 3 advantages:
 - `aspawn` allows user to **do anything** in the child process before `exec`.
 - `aspawn` can reuse stack, `posix_spawn` can't;
 - `aspawn` doesn't block the parent thread;

## Benchmark

### Responsive benchmark

Responsive comparison between `posix_spawn` and `aspawn`, [source code][5]:

```
2020-09-26T20:22:01+10:00
Running ./bench_aspawn_responsiveness.out
Run on (12 X 4100 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x6)
  L1 Instruction 32 KiB (x6)
  L2 Unified 256 KiB (x6)
  L3 Unified 9216 KiB (x1)
Load Average: 0.45, 0.47, 0.46
--------------------------------------------------------------
Benchmark                    Time             CPU   Iterations
--------------------------------------------------------------
BM_aspawn_no_reuse       14221 ns        14018 ns        49346
BM_aspawn/threads:1      10292 ns        10103 ns        68454
BM_posix_spawn          150883 ns        42476 ns        16474

```

### Throughput benchmark

Since `aspawn` allows user to **do anything** in the vforked child via `aspawn_fn`, it makes no sense
to benchmark how many processes can `aspawn` created as it depends on user provided argument `fn`.

[1]: https://github.com/NobodyXu/aspawn
[2]: https://github.com/NobodyXu/aspawn/blob/master/syscall/syscall.h
[3]: https://man7.org/linux/man-pages/man3/posix_spawn.3.html
[4]: https://gist.github.com/nicowilliams/a8a07b0fc75df05f684c23c18d7db234
[5]: https://github.com/NobodyXu/aspawn/blob/master/benchmark/bench_aspawn_responsiveness.cc
