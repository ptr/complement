/*  Time-stamp: <05/12/12 10:46:44 ptr> */

/*
 *
 * Copyright (c) 2003, 2005
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 2.1
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 *
 */

#ifndef __config__freebsd_h
#define __config__freebsd_h

/* Endiannes */
#include <sys/types.h>

#define __FIT__P_PROBLEM /* Hide __P from sys/cdefs.h. Workaround for glibc. */

#if defined(_REENTRANT) && !defined(_PTHREADS)
#  define _PTHREADS
#endif

#if defined(_PTHREADS)
#  ifndef __USE_UNIX98
#    define __USE_UNIX98
#  endif
#  define __FIT_XSI_THR  /* Unix 98 or X/Open System Interfaces Extention */
/* The IEEE Std. 1003.1j-2000 introduces functions to implement spinlocks. */
#  if __FreeBSD_version >= 503001
#    define __FIT_PTHREAD_SPINLOCK
#  endif
#  define __FIT_RWLOCK
#endif

/*
#if !defined(BYTE_ORDER) || !defined(LITTLE_ENDIAN) || !defined(BIG_ENDIAN)
#  error "One of __BYTE_ORDER, __LITTLE_ENDIAN and __BIG_ENDIAN undefined; Fix me!"
#endif

#if (BYTE_ORDER == LITTLE_ENDIAN)
#  define _LITTLE_ENDIAN
#elif (BYTE_ORDER == BIG_ENDIAN)
#  define _BIG_ENDIAN
#else
#  error "BYTE_ORDER neither BIG_ENDIAN nor LITTLE_ENDIAN; Fix me!"
#endif
*/

/* No gethostbyname_r and gethostbyaddr_r functions */
#define __FIT_GETHOSTBYADDR

#if (__FreeBSD_version < 503001)
/* use workaround for non-reentrant res_*, ns_*, getaddrinfo functions */
#  define __FIT_NONREENTRANT
#endif

#endif /* __config__freebsd_h */
