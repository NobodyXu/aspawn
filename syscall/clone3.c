#include "clone3.h"
#include "make_syscall.h"
#include "syscall.h"

#include <sys/syscall.h>

long psys_clone3(struct psys_clone_args *cl_args, size_t size, int (*fn)(void *arg), void *arg)
{
    typedef int (*fn_t)(void *arg);

    register fn_t fn_reg __asm__ ("r13");
    register void *arg_reg __asm__ ("r12");

    fn_reg = fn;
    arg_reg = arg;

    // invoke_syscall is always inline, thus no need of worring about the affect of switching to a new stack,
    // since no function return (req + pop) will be performed, and the return value is in the register.
    //
    // The compiler probably won't spoil the return value from the register to the stack.
    int result = INTERNAL_SYSCALL(SYS_clone3, 2, cl_args, size);

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
    }

    return result;
}
