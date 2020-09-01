#ifndef STACK_GROWS_DOWN

# if defined(__hppa__) || defined(__ia64__)
/**
 * STACK_GROWS_DOWN is a boolean, indicates whether stack grows down on this platform.
 */
#  define STACK_GROWS_DOWN 0
# else
#  define STACK_GROWS_DOWN 1
# endif
#endif
