#include "syscall.h"

void pmemset(void *s, int c, size_t n)
{
    char *p = s;
    for (size_t i = 0; i != n; ++i)
        p[i] = c;
}

void pmemcpy(void *dest, const void *src, size_t n)
{
    char *destp = dest;
    const char *srcp = src;

    for (size_t i = 0; i != n; ++i)
        destp[i] = srcp[i];
}
