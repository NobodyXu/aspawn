#ifndef make_syscall
# ifdef __x86_64__

/**
 * The code below is copied from glibc
 */

/* Registers clobbered by syscall.  */
# define REGISTERS_CLOBBERED_BY_SYSCALL "cc", "r11", "cx"

/* NB: This also works when X is an array.  For an array X,  type of
   (X) - (X) is ptrdiff_t, which is signed, since size of ptrdiff_t
   == size of pointer, cast is a NOP.   */
# define TYPEFY1(X) __typeof__ ((X) - (X))
/* Explicit cast the argument.  */
# define ARGIFY(X) ((TYPEFY1 (X)) (X))
/* Create a variable 'name' based on type of variable 'X' to avoid
   explicit types.  */
# define TYPEFY(X, name) __typeof__ (ARGIFY (X)) name

# undef INTERNAL_SYSCALL
# define INTERNAL_SYSCALL(number, nr, args...)				\
	internal_syscall##nr (number, args)

# undef internal_syscall0
# define internal_syscall0(number, dummy...)				\
({									\
    unsigned long int resultvar;					\
    __asm__ __volatile__ (							\
    "syscall\n\t"							\
    : "=a" (resultvar)							\
    : "0" (number)							\
    : "memory", REGISTERS_CLOBBERED_BY_SYSCALL);			\
    (long int) resultvar;						\
})

# undef internal_syscall1
# define internal_syscall1(number, arg1)					\
({									\
    unsigned long int resultvar;					\
    TYPEFY (arg1, __arg1) = ARGIFY (arg1);			 	\
    register TYPEFY (arg1, _a1) __asm__ ("rdi") = __arg1;			\
    __asm__ __volatile__ (							\
    "syscall\n\t"							\
    : "=a" (resultvar)							\
    : "0" (number), "r" (_a1)						\
    : "memory", REGISTERS_CLOBBERED_BY_SYSCALL);			\
    (long int) resultvar;						\
})

# undef internal_syscall2
# define internal_syscall2(number, arg1, arg2)				\
({									\
    unsigned long int resultvar;					\
    TYPEFY (arg2, __arg2) = ARGIFY (arg2);			 	\
    TYPEFY (arg1, __arg1) = ARGIFY (arg1);			 	\
    register TYPEFY (arg2, _a2) __asm__ ("rsi") = __arg2;			\
    register TYPEFY (arg1, _a1) __asm__ ("rdi") = __arg1;			\
    __asm__ __volatile__ (							\
    "syscall\n\t"							\
    : "=a" (resultvar)							\
    : "0" (number), "r" (_a1), "r" (_a2)				\
    : "memory", REGISTERS_CLOBBERED_BY_SYSCALL);			\
    (long int) resultvar;						\
})

# undef internal_syscall3
# define internal_syscall3(number, arg1, arg2, arg3)			\
({									\
    unsigned long int resultvar;					\
    TYPEFY (arg3, __arg3) = ARGIFY (arg3);			 	\
    TYPEFY (arg2, __arg2) = ARGIFY (arg2);			 	\
    TYPEFY (arg1, __arg1) = ARGIFY (arg1);			 	\
    register TYPEFY (arg3, _a3) __asm__ ("rdx") = __arg3;			\
    register TYPEFY (arg2, _a2) __asm__ ("rsi") = __arg2;			\
    register TYPEFY (arg1, _a1) __asm__ ("rdi") = __arg1;			\
    __asm__ __volatile__ (							\
    "syscall\n\t"							\
    : "=a" (resultvar)							\
    : "0" (number), "r" (_a1), "r" (_a2), "r" (_a3)			\
    : "memory", REGISTERS_CLOBBERED_BY_SYSCALL);			\
    (long int) resultvar;						\
})

# undef internal_syscall4
# define internal_syscall4(number, arg1, arg2, arg3, arg4)		\
({									\
    unsigned long int resultvar;					\
    TYPEFY (arg4, __arg4) = ARGIFY (arg4);			 	\
    TYPEFY (arg3, __arg3) = ARGIFY (arg3);			 	\
    TYPEFY (arg2, __arg2) = ARGIFY (arg2);			 	\
    TYPEFY (arg1, __arg1) = ARGIFY (arg1);			 	\
    register TYPEFY (arg4, _a4) __asm__ ("r10") = __arg4;			\
    register TYPEFY (arg3, _a3) __asm__ ("rdx") = __arg3;			\
    register TYPEFY (arg2, _a2) __asm__ ("rsi") = __arg2;			\
    register TYPEFY (arg1, _a1) __asm__ ("rdi") = __arg1;			\
    __asm__ __volatile__ (							\
    "syscall\n\t"							\
    : "=a" (resultvar)							\
    : "0" (number), "r" (_a1), "r" (_a2), "r" (_a3), "r" (_a4)		\
    : "memory", REGISTERS_CLOBBERED_BY_SYSCALL);			\
    (long int) resultvar;						\
})

# undef internal_syscall5
# define internal_syscall5(number, arg1, arg2, arg3, arg4, arg5)	\
({									\
    unsigned long int resultvar;					\
    TYPEFY (arg5, __arg5) = ARGIFY (arg5);			 	\
    TYPEFY (arg4, __arg4) = ARGIFY (arg4);			 	\
    TYPEFY (arg3, __arg3) = ARGIFY (arg3);			 	\
    TYPEFY (arg2, __arg2) = ARGIFY (arg2);			 	\
    TYPEFY (arg1, __arg1) = ARGIFY (arg1);			 	\
    register TYPEFY (arg5, _a5) __asm__ ("r8") = __arg5;			\
    register TYPEFY (arg4, _a4) __asm__ ("r10") = __arg4;			\
    register TYPEFY (arg3, _a3) __asm__ ("rdx") = __arg3;			\
    register TYPEFY (arg2, _a2) __asm__ ("rsi") = __arg2;			\
    register TYPEFY (arg1, _a1) __asm__ ("rdi") = __arg1;			\
    __asm__ __volatile__ (							\
    "syscall\n\t"							\
    : "=a" (resultvar)							\
    : "0" (number), "r" (_a1), "r" (_a2), "r" (_a3), "r" (_a4),		\
      "r" (_a5)								\
    : "memory", REGISTERS_CLOBBERED_BY_SYSCALL);			\
    (long int) resultvar;						\
})

# undef internal_syscall6
# define internal_syscall6(number, arg1, arg2, arg3, arg4, arg5, arg6) \
({									\
    unsigned long int resultvar;					\
    TYPEFY (arg6, __arg6) = ARGIFY (arg6);			 	\
    TYPEFY (arg5, __arg5) = ARGIFY (arg5);			 	\
    TYPEFY (arg4, __arg4) = ARGIFY (arg4);			 	\
    TYPEFY (arg3, __arg3) = ARGIFY (arg3);			 	\
    TYPEFY (arg2, __arg2) = ARGIFY (arg2);			 	\
    TYPEFY (arg1, __arg1) = ARGIFY (arg1);			 	\
    register TYPEFY (arg6, _a6) __asm__ ("r9") = __arg6;			\
    register TYPEFY (arg5, _a5) __asm__ ("r8") = __arg5;			\
    register TYPEFY (arg4, _a4) __asm__ ("r10") = __arg4;			\
    register TYPEFY (arg3, _a3) __asm__ ("rdx") = __arg3;			\
    register TYPEFY (arg2, _a2) __asm__ ("rsi") = __arg2;			\
    register TYPEFY (arg1, _a1) __asm__ ("rdi") = __arg1;			\
    __asm__ __volatile__ (							\
    "syscall\n\t"							\
    : "=a" (resultvar)							\
    : "0" (number), "r" (_a1), "r" (_a2), "r" (_a3), "r" (_a4),		\
      "r" (_a5), "r" (_a6)						\
    : "memory", REGISTERS_CLOBBERED_BY_SYSCALL);			\
    (long int) resultvar;						\
})

# endif
#endif
