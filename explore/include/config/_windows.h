// -*- C++ -*- Time-stamp: <03/01/19 13:23:03 ptr>

/*
 *
 * Copyright (c) 2003
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 1.0
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

#ifndef __config__windows_h
#define __config__windows_h

// "@(#)$Id$"

#if !defined(__FIT_USE_STATIC_LIB) && !defined(_LIB)
#  undef  __FIT_USE_DECLSPEC
#  define __FIT_USE_DECLSPEC
#else
#  undef  __FIT_USE_DECLSPEC
#endif

#if (defined (__DLL) || defined (_DLL) || defined (_WINDLL) )
#  undef  __FIT_DLL
#  define __FIT_DLL
#else 
#  undef  __FIT_DLL
#endif

#if !defined(_PTHREADS)
#  define __FIT_WIN32THREADS
#endif

#define __FIT_SELECT // use select system call

// *** Endians issues

// #include <winsock.h>
// #  if defined(BIGENDIAN) && (BIGENDIAN > 0)
// #    define _BIG_ENDIAN
// #  else
#define _LITTLE_ENDIAN // Wins run only on Intel
// #  endif

#endif // __config___windows_h
