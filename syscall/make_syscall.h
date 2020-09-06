#ifndef make_syscall
# ifdef __x86_64__
#  define make_syscall()                                           \
    __asm__  __volatile__ (                                        \
             "movq %%rdi, %%rax\n\t" /* Syscall number -> rax.  */ \
             "movq %%rsi, %%rdi\n\t" /* shift arg1 - arg5 */       \
             "movq %%rdx, %%rsi\n\t"                               \
             "movq %%rcx, %%rdx\n\t"                               \
             "movq %%r8, %%r10\n\t"                                \
             "movq %%r9, %%r8\n\t"                                 \
             "movq 8(%%rsp), %%r9\n\t" /* arg6 is on the stack */  \
             "syscall"                                             \
        :                                                          \
        :                                                          \
        : "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9")

# endif
#endif
