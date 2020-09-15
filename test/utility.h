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

#endif
