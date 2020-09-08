# if __GNUC__ >= 4
#  define PUBLIC __attribute__ ((visibility ("default")))
#  define LOCAL  __attribute__ ((visibility ("hidden")))
#  define ALWAYS_INLINE __attribute__ ((always_inline))
# else
#  define PUBLIC
#  define LOCAL
# endif
