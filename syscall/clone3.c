#include "clone3.h"
#include "make_syscall.h"

#include <sys/syscall.h>

__attribute__((always_inline)) 
long invoke_syscall(long syscall_number, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6)
{
    make_syscall();
    __asm__ __volatile__ ("ret");
    __builtin_unreachable();
}

__attribute__((always_inline)) long psys_clone3(struct clone_args *cl_args, size_t size)
{
    return invoke_syscall(SYS_clone3, (long) cl_args, size, 0, 0, 0, 0);
}
