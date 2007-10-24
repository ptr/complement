/* Time-stamp: <07/01/31 23:51:12 ptr> */

/*
 *
 * Copyright (c) 2003-2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __config__linux_h
#define __config__linux_h

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

/*
 * Include this first, due to <features.h> unconditionally redefine
 * a lot of macros.
 */

#include <features.h>

#define __FIT__P_PROBLEM /* Hide __P from sys/cdefs.h. Workaround for glibc. */

#if defined(_REENTRANT) && !defined(_PTHREADS)
# define _PTHREADS
#endif

#if defined(_PTHREADS)
/*
#  ifndef __USE_UNIX98
#    define __USE_UNIX98
#  endif
*/
/* This feature exist at least since glibc 2.2.4 */
#  define __FIT_XSI_THR  /* Unix 98 or X/Open System Interfaces Extention */
#  ifdef __USE_XOPEN2K
/* The IEEE Std. 1003.1j-2000 introduces functions to implement spinlocks. */
#   ifndef __UCLIBC__ /* There are no spinlocks in uClibc 0.9.27 */
#     define __FIT_PTHREAD_SPINLOCK
#     define __FIT_RWLOCK
#   endif
#   define __FIT_PSHARED_MUTEX
#  endif
#endif

/* Endiannes */
#include <sys/types.h>
#if !defined(__BYTE_ORDER) || !defined(__LITTLE_ENDIAN) || !defined(__BIG_ENDIAN)
#  error "One of __BYTE_ORDER, __LITTLE_ENDIAN and __BIG_ENDIAN undefined; Fix me!"
#endif

#if ( __BYTE_ORDER == __LITTLE_ENDIAN )
#  define _LITTLE_ENDIAN
#elif ( __BYTE_ORDER == __BIG_ENDIAN )
#  define _BIG_ENDIAN
#else
#  error "__BYTE_ORDER neither __BIG_ENDIAN nor __LITTLE_ENDIAN; Fix me!"
#endif

/*
 * select-based socket manager not maintained a long time, and produce
 * errors on x86_64, so I turn off usage of select
 */

#define __FIT_NO_SELECT

/*
 * use algorithms that based on non-block sockets technique
 */
/*
#define __FIT_NONBLOCK_SOCKETS
*/
/*
 * use epoll syscall instead of poll
 */

/*
#define __FIT_EPOLL
*/

/*
 * Normally, in Linux present BFD mechanism
 */

#ifndef __FIT_DISABLE_BFD
#  define __FIT_PRESENT_BFD
#endif

#endif /* __config__linux_h */
