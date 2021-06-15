# aspawn

![C/C++ CI](https://github.com/NobodyXu/aspawn/workflows/C/C++%20CI/badge.svg)

Asynchronous and more versatile replacement of posix_spawn

## Intro

`aspawn` implements the idea from [`avfork`][4], enabling user to do `posix_spawn` in an asynchronous way by
making syscall directly via `pure_syscall` implemented in my library that does not use
any global/thread local variable at all.

My `aspawn` has signature:

```c
struct Stack_t {
    void *addr;
    size_t size;
};

/**
 * @param old_sigset of type sigset_t*. The original value of sigmask.
 *                   It can be modified to any value user desired.
 *
 * The value of sigmask in aspawn_fn is unspecified.
 */
typedef int (*aspawn_fn)(void *arg, int wirte_end_fd, void *old_sigset);

/**
 * @param pid the pid of the child will be stored into it on success.
 * @param cached_stack must call reserve_stack before using aspawn
 * @param fn If fn returns, then the child will exit with the return value of fn as the exit code.
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
PUBLIC int aspawn(pid_t *pid, struct Stack_t *cached_stack, aspawn_fn fn, void *arg);
```

By returning the write end of the `CLOEXEC` pipefd, user of this library is able to receive error message/check whether
the child process has done using `cached_stack` so that `aspawn` can reuse `cached_stack`.

It also allows user to pass arbitary data in the stack via `allocate_obj_on_stack`, 
thus user does not have to allocate them separately on heap.

To use a syscall, you need to include [`syscall/syscall.h`][2], which defines the syscall routine used by the child process including
`find_exe`, `psys_execve` and `psys_execveat`.

User will be able to reuse stack by `poll`ing the fd returned by `aspawn` and wait for it to hup.

Compare to [`posix_spawn`][3], `aspawn` has 3 advantages:
 - `aspawn` allows user to **do anything** in the child process before `exec`.
 - `aspawn` can reuse stack, `posix_spawn` can't;
 - `aspawn` doesn't block the parent thread;

### Example code

Examples can be seen [here][10]
 
### Platform support

Currently, it only supports x86-64 modern linux (>= 4.0), but ports can be easily made by modifing [`syscall/`][7],
[`create_pipe/create_pipe.c`][8] and [`clone_internal/`][9]

## Benchmark

### Responsive benchmark

Responsive comparison between `posix_spawn` and `aspawn`, [source code][5] (benchmarking is done via [google/benchmark][6]:

```console
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

The column "Time" is measured in terms of system clock, while "CPU" is measured in terms of per-process CPU time.

### Throughput benchmark

Since `aspawn` allows user to **do anything** in the vforked child via `aspawn_fn`, it makes no sense
to benchmark how many processes can `aspawn` created as it depends on user provided argument `fn`.

## Build and Install

Make sure that you have installed `make`, `clang` `lld` and `llvm-ar`.

Then run `make -j $(nproc)` to build the project, `sudo make install` to install project to `/usr/local/`.

## Testing

Make sure you have installed all depedencies listed above for building this project,
then run `make test -j $(nproc)`

## Contributing to this project

Any commits on this project will be welcome!

It would be even better if you can help me improve test coverages by adding more unit tests or port this project to other platform (e.g. `arm`, `mips`).

## Contributors

Thank you for people who contributed to this project:
 - @HappyFacade

[1]: https://github.com/NobodyXu/aspawn
[2]: https://github.com/NobodyXu/aspawn/blob/master/syscall/syscall.h
[3]: https://man7.org/linux/man-pages/man3/posix_spawn.3.html
[4]: https://gist.github.com/nicowilliams/a8a07b0fc75df05f684c23c18d7db234
[5]: https://github.com/NobodyXu/aspawn/blob/master/benchmark/bench_aspawn_responsiveness.cc
[6]: https://github.com/google/benchmark
[7]: https://github.com/NobodyXu/aspawn/tree/master/syscall
[8]: https://github.com/NobodyXu/aspawn/blob/master/create_pipe/create_pipe.c
[9]: https://github.com/NobodyXu/aspawn/tree/master/clone_internal
[10]: https://github.com/NobodyXu/aspawn/tree/master/example
