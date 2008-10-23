/* Time-stamp: <08/02/20 10:02:49 ptr> */

/*
 *
 * Copyright (c) 2003-2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __config__windows_h
#define __config__windows_h

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
#define __FIT_NO_POLL

/* *** Endians issues */

/*
#include <winsock.h>
#  if defined(BIGENDIAN) && (BIGENDIAN > 0)
#    define _BIG_ENDIAN
#  else
*/
#define _LITTLE_ENDIAN /* Wins run only on Intel */
/*#  endif */

/* For WIN32 gethostbyaddr is reeentrant */
#define __FIT_GETHOSTBYADDR

#endif /* __config___windows_h */
