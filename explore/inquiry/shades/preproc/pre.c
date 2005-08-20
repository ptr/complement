#define a b
a
#define c d
#define e c
e
#define x y
#define z x
#undef x
z

extern int errno;
extern int *errno1 (void);
#define errno (*errno1 ())
<errno.h>

// #define _STLP_MAKE_HEADER(slash,path, header) <path##slash##header>
//#define _STLP_MAKE_HEADER(path, header) <path/header>
#define X(p,h) <GLUE(p,h)>
#define GLUE(p,h) p/h
#define _STLP_MAKE_HEADER(path, header) GLUE(GLUE(GLUE(<,path),/),header)
#define  X_PATH ../include

//#ifdef errno
//# define __save_errno errno
//# undef errno
//#endif

//_STLP_MAKE_HEADER(/,X_PATH,errno.h)
//_STLP_MAKE_HEADER(X_PATH,errno.h)
X(X_PATH,errno.h)
#define errno errno2

//#if defined(errno) || defined(__save_errno) 
//# ifdef __save_errno
//#   define errno __save_errno
/* #   undef __save_errno */
//# endif
//#endif

errno
