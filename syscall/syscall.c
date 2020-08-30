#include "syscall.h"

long syscall(long syscall_number, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6)
{
#ifdef __x86_64__
    __asm__  __volatile__ (
             "movq %%rdi, %%rax\n\t" /* Syscall number -> rax.  */
             "movq %%rsi, %%rdi\n\t" /* shift arg1 - arg5 */
             "movq %%rdx, %%rsi\n\t"
             "movq %%rcx, %%rdx\n\t"
             "movq %%r8, %%r10\n\t"
             "movq %%r9, %%r8\n\t"
             "movq 8(%%rsp), %%r9\n\t" /* arg6 is on the stack */
             "syscall\n\t"
             "ret"
        :
        :
        : "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9");
#endif

    __builtin_unreachable();
}
