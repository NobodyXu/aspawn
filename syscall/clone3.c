#include "clone3.h"
#include "make_syscall.h"
#include "syscall.h"

#include <sys/syscall.h>
#include <errno.h>

long psys_clone3(struct psys_clone_args *cl_args, size_t size, int (*fn)(void *arg), void *arg)
{
#ifdef SYS_clone3
    typedef int (*fn_t)(void *arg);

    // TODO: Remove manual register allocation as compiler knows exactly
    // which register needs to spoil for the syscall
    register fn_t fn_reg __asm__ ("r13");
    register void *arg_reg __asm__ ("r12");
    register int result __asm__ ("rax");

    fn_reg = fn;
    arg_reg = arg;

    // invoke_syscall is always inline, thus no need of worring about the affect of switching to a new stack,
    // since no function return (req + pop) will be performed, and the return value is in the register.
    //
    // The compiler probably won't spoil the return value from the register to the stack.
    result = INTERNAL_SYSCALL(SYS_clone3, 2, cl_args, size);

    // If this is the child
    if (result == 0) {
        __asm__ __volatile__ (
	        // Clear the frame pointer.
            // This is suggested by the API to be done to mark the outermost frame.
            "xorl %%ebp, %%ebp"
            :
            :
            : "ebp"
        );

        int exit_status = fn_reg(arg_reg);
        psys_exit(exit_status);
        __builtin_unreachable();
    }

    return result;
#else
    return -ENOSYS;
#endif
}
