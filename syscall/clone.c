#include "clone.h"
#include "make_syscall.h"
#include "syscall.h"

#include <sys/syscall.h>
#include <stdint.h>

/**
 *
 * __clone(func, stack, flags, arg, ptid, tls, ctid)
 * # %rdi, %rsi, %rdx, %rcx, %r8, and %r9
 *
 * 	xor %eax,%eax
 *
	mov $56,%al # syscall_number

	mov 3rd arg,%rdi     # syscall arg1

    # rsi is the 2st arg
    and $-16,%rsi # syscall arg2
	sub $8,%rsi
	mov 4th arg,(%rsi)

	mov 5th arg,%rdx # syscall arg3
	mov 7th arg,%r10 # syscall arg4
	mov 6th arg,%r8  # syscall arg5

    mov 1st arg, syscall arg6 # I believe this isn't part of syscall argument

	syscall

    if (eax == 0) {
	    xor %ebp,%ebp

	    mov %rcx (arg), %rdi
	    call fp

	    mov %eax,%edi
	    xor %eax,%eax
	    mov $60,%al
	    syscall
	    hlt
    }
    ret

 */

int psys_clone(int (*fn)(void *arg), void *stack, int flags, void *arg,
               pid_t *parent_tid, void *tls, pid_t *child_tid)
{
    typedef int (*fn_t)(void *arg);

    register fn_t fn_reg __asm__ ("r13");
    register void *arg_reg __asm__ ("r12");
    register int result __asm__ ("rax");

    fn_reg = fn;
    arg_reg = arg;

    //stack = (void*) (((uintptr_t) stack) & ((uintptr_t) -16));

    // invoke_syscall is always inline, thus no need of worring about the affect of switching to a new stack,
    // since no function return (req + pop) will be performed, and the return value is in the register.
    result = INTERNAL_SYSCALL(SYS_clone, 5, flags, stack, parent_tid, child_tid, tls);

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
}
