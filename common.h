# if __GNUC__ >= 4
#  define PUBLIC __attribute__ ((visibility ("default")))
#  define LOCAL  __attribute__ ((visibility ("hidden")))
# else
#  define PUBLIC
#  define LOCAL
# endif
