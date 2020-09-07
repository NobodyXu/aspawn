#define _POSIX_C_SOURCE 201107L

#include "syscall.h"
#include "make_syscall.h"
#include <sys/syscall.h>

#include <string.h>
#include <signal.h>

void pure_sigemptyset(void *set)
{
    pmemset(set, 0, sizeof(sigset_t));
}
void pure_sigfillset(void *set)
{
    pmemset(set, -1, sizeof(sigset_t));
}

/* This is the sigaction structure from the Linux 3.2 kernel.  */
struct psys_kernel_sigaction {
    void (*psys_sa_handler)(int);
    unsigned long psys_sa_flags;
    void (*psys_sa_restorer) (void);
    /* glibc sigset is larger than kernel expected one, however sigaction
       passes the kernel expected size on rt_sigaction syscall.  */
    sigset_t sa_mask;
};

#define NSIG (SIGRTMAX + 1)

#define UCHAR_WIDTH 8
#define ULONG_WIDTH (sizeof(unsigned long) * UCHAR_WIDTH)

#define ALIGN_DOWN(base, size)	((base) & -((__typeof__ (base)) (size)))
#define ALIGN_UP(base, size)	ALIGN_DOWN ((base) + (size) - 1, (size))

#define NSIG_WORDS (ALIGN_UP((NSIG - 1), ULONG_WIDTH) / ULONG_WIDTH)
#define NSIG_BYTES (NSIG_WORDS * (ULONG_WIDTH / UCHAR_WIDTH))

int psys_sig_set_handler(int signum, int ignore)
{
    struct psys_kernel_sigaction act;

    act.psys_sa_handler = ignore ? SIG_IGN : SIG_DFL;
    act.psys_sa_flags = 0;
    act.psys_sa_restorer = 0;
    pure_sigemptyset(&act.sa_mask);

    return INTERNAL_SYSCALL(SYS_rt_sigaction, 4, signum, &act, NULL, NSIG_BYTES);
}

int psys_sigprocmask(int how, const void *set, void *oldset)
{
    return INTERNAL_SYSCALL(SYS_rt_sigprocmask, 4, how, set, oldset, NSIG_BYTES);
}
