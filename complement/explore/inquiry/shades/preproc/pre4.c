extern int errno;
extern int *__errno_location (void) __attribute__ ((__const__));
#   define errno (*__errno_location ())
errno
#if 0
<errno.h>
#endif

/* #include <errno.h> */
/* extern int errno; */

/* extern int *__errno_location (void) __attribute__ ((__const__)); */

/* #   define errno (*__errno_location ()) */

errno

#define INCL(f) ../include/f

#include INCL((errno.h))

#if 0
#include <../include/errno.h>
#define xstr(s) #s
#define X(p,f) p ## f
/* #include xstr(X(../include,errno.h)) */
#include xstr(../include/errno.h)
#define str(s) # s
#define xstr(s) str(s)
#define INCL(f,n) f ## n
xstr(INCL(../include/er,nno.h))
#include xstr(INCL(../include/er,nno.h))
#endif

#define file xfile
#define str(s) # s
#define ystr(s) s
#define path xpath
#define xstr(s) <path/s>
#define zfile file
#undef file
xstr(file.h)
#define file zfile
file
