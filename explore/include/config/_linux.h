/* -*- C++ -*- Time-stamp: <04/05/18 08:56:40 ptr> */

/*
 *
 * Copyright (c) 2003, 2004
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 2.0
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

#ifndef __config__linux_h
#define __config__linux_h

#ident "@(#)$Id$"

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
#   define __FIT_PTHREAD_SPINLOCK
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

#endif /* __config__linux_h */
