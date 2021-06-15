#ifndef  __aspawn_test_utility_H__
# define __aspawn_test_utility_H__

# include <assert.h>
# include <stddef.h>

# define ASSERT_SYSCALL(expr)                                           \
    ({                                                                  \
        int result = (expr);                                            \
        if (result < 0)                                                 \
            err(1, "%s on line %zu failed", # expr, (size_t) __LINE__); \
        result;                                                         \
     })

void assert_aspawnf_internal(int result, const char *msg);

# define ASSERT_ASPAWNF(expr)    \
    ({                           \
        int ret = (expr);        \
        assert_aspawnf_internal( \
            ret,                 \
            # expr "failed"      \
        );                       \
        ret;                     \
     })

int test_aspawn_fn(void *arg, int write_end_fd, void *old_sigset);

#endif
