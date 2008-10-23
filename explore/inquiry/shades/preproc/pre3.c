/* #include <errno.h> */
extern int errno;

extern int *__errno_location (void) __attribute__ ((__const__));

#   define errno (*__errno_location ())

errno

#include <errno.h>
