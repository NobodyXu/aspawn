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
$ ll -h bench_aspawn_responsiveness.out
-rwxrwxr-x 1 nobodyxu nobodyxu 254K Oct  2 15:02 bench_aspawn_responsiveness.out*

$ uname -a
Linux pop-os 5.4.0-7642-generic #46~1598628707~20.04~040157c-Ubuntu SMP Fri Aug 28 18:02:16 UTC  x86_64 x86_64 x86_64 GNU/Linux

$ ./a.out
2020-10-02T15:02:45+10:00
Running ./bench_aspawn_responsiveness.out
Run on (12 X 4100 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x6)
  L1 Instruction 32 KiB (x6)
  L2 Unified 256 KiB (x6)
  L3 Unified 9216 KiB (x1)
Load Average: 0.31, 0.36, 0.32
---------------------------------------------------------------------
Benchmark                           Time             CPU   Iterations
---------------------------------------------------------------------
BM_aspawn_no_reuse              18009 ns        17942 ns        38943
BM_aspawn/threads:1             14500 ns        14446 ns        48339
BM_vfork_with_shared_stack      46545 ns        16554 ns        44027
BM_fork                         54583 ns        54527 ns        12810
BM_posix_spawn                 125061 ns        29091 ns        24483
```

### Throughput benchmark

Since `aspawn` allows user to **do anything** in the vforked child via `aspawn_fn`, it makes no sense
to benchmark how many processes can `aspawn` created as it depends on user provided argument `fn`.

## Build and Install

Make sure that you have installed `make`, `clang` `lld` and `llvm-ar`.

Then run `make -j $(nproc)` to build the project, `sudo make install` to install project to `/usr/local/`.

## Testing

Make sure you have installed all depedencies listed above for building this project,
then run `make test -j $(nproc)`

[1]: https://github.com/NobodyXu/aspawn
[2]: https://github.com/NobodyXu/aspawn/blob/master/syscall/syscall.h
[3]: https://man7.org/linux/man-pages/man3/posix_spawn.3.html
[4]: https://gist.github.com/nicowilliams/a8a07b0fc75df05f684c23c18d7db234
[5]: https://github.com/NobodyXu/aspawn/blob/master/benchmark/bench_aspawn_responsiveness.cc
