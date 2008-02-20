// -*- C++ -*- Time-stamp: <08/02/20 10:01:25 ptr>

/*
 *
 * Copyright (c) 2003-2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __config__openbsd_h
#define __config__openbsd_h

#define __FIT__P_PROBLEM // Hide __P from sys/cdefs.h. Workaround for glibc.

#ifndef __unix
# define __unix
#endif

#if defined(_REENTRANT) && !defined(_PTHREADS)
#  define _PTHREADS
#endif

#if defined(_PTHREADS)
#  define __FIT_PTHREADS
#  ifndef __USE_UNIX98
#    define __USE_UNIX98
#  endif
// This feature exist at least since glibc 2.2.4
#  define __FIT_XSI_THR  // Unix 98 or X/Open System Interfaces Extention
#  ifdef __USE_XOPEN2K
// The IEEE Std. 1003.1j-2000 introduces functions to implement spinlocks.
#   define __FIT_PTHREAD_SPINLOCK
#  endif
#endif

// Endiannes
#include <sys/types.h>
// #if !defined(BYTE_ORDER) || !defined(LITTLE_ENDIAN) || !defined(BIG_ENDIAN)
// #  error "One of __BYTE_ORDER, __LITTLE_ENDIAN and __BIG_ENDIAN undefined; Fix me!"
// #endif

// #if (BYTE_ORDER == LITTLE_ENDIAN)
// #  define _LITTLE_ENDIAN
// #elif (BYTE_ORDER == BIG_ENDIAN)
// #  define _BIG_ENDIAN
// #else
// #  error "BYTE_ORDER neither BIG_ENDIAN nor LITTLE_ENDIAN; Fix me!"
// #endif

#endif // __config__openbsd_h
