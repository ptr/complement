// -*- C++ -*- Time-stamp: <02/08/01 09:58:50 ptr>

/*
 *
 * Copyright (c) 1999, 2002
 * Petr Ovtchenkov
 *
 * Copyright (c) 1999-2001
 * Parallel Graphics Ltd.
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */
#ifndef __config_feature_h
#define __config_feature_h

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#pragma ident "@(#)$Id$"
#  endif
#endif

// #define __USE_SGI_STL_PORT  0x400

// #if /* defined( __STL_DEBUG ) && */ (__USE_SGI_STL_PORT >= 0x400)
// // #  define __STD __stl_native_std
// #  define __STD __STLPORT_STD // __std_alias
// #  ifdef __STL_DEBUG 
// #    define __VENDOR_STD  __STL_VENDOR_EXCEPT_STD
// #  else
// #    define __VENDOR_STD  __std_alias
// #  endif
// #endif


#ifndef _REENTRANT
#  define _REENTRANT
#endif

#if defined( WIN32 ) && !defined(__FIT_USE_STATIC_LIB) && !defined(_LIB)
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

#ifdef __FIT_USE_DECLSPEC /* using export/import technique */
#  ifdef __FIT_DLL
#    define __FIT_DECLSPEC   __declspec( dllexport )
#  else
#    define __FIT_DECLSPEC   __declspec( dllimport )
#  endif
#else
#  define __FIT_DECLSPEC
#endif

#if defined(__sun) && defined(__GNUC__)
#  define __FIT_USE_ABBREV
#endif

/*
 * UNIX 95 implementation
 *
 * As specified in the following X/Open specifications:
 *
 *   System Interfaces and Headers, Issue 4, Version 2
 *   Commands and Utilities, Issue 4, Version 2
 *   Networking Services, Issue 4
 *   X/Open Curses, Issue 4
 *
 */

/*
 *   This code compliant with XPG, Issue 4, Version 2
 *   (under Solaris 5.6 and Solaris 7) with some extentions
 */

#ifdef __sun // __SunOS_5_6, __SunOS_5_7
#  define _XOPEN_SOURCE 1
#  define _XOPEN_SOURCE_EXTENDED 1
#  define __EXTENSIONS__ 1
#  define __XPG4v2 1
#endif // __SunOS_5_6

#ifdef __hpux
#  define _XOPEN_SOURCE_EXTENDED 1
#  define _INCLUDE_AES_SOURCE
#  ifndef _PTHREADS
#    define _PTHREADS
#  endif
#endif

#ifdef __SUNPRO_CC
#  ifndef __LINK_TIME_INSTANTIATION
#    define __LINK_TIME_INSTANTIATION 1
#  endif
#  define __FIT_INCLASS_OPERATOR
#endif // __SUNPRO_CC

#if defined(__sun) && !defined(_PTHREADS)
#  define __FIT_UITHREADS
#endif

#if defined(linux)
# define __FIT__P_PROBLEM
#endif // Hide __P from sys/cdefs.h. Workaround for glibc.

#if defined(linux) && defined(_REENTRANT) && !defined(_PTHREADS)
#  define _PTHREADS
#endif

#if defined(WIN32) && !defined(_PTHREADS)
#  define __FIT_WIN32THREADS
#endif

#if defined(__HP_aCC) && (__HP_aCC==1) // aCC A.03.13
#  define __FIT_TEMPLATE_FORWARD_BUG
#  define __FIT_EXPLICIT_BUG
#  define __FIT_USING_NAMESPACE_BUG
// #  define __FIT_TYPENAME_TEMPLATE_PARAMETER_BUG
#  define __FIT_TYPENAME_TEMPLATE_PARAMETER
#  define __FIT_NEED_TYPENAME_IN_ARGS_BUG
#  define __FIT_TEMPLATE_TYPEDEF_BUG
#  define __FIT_TEMPLATE_CLASSTYPE_BUG
#endif

// #if defined(__GNUC__) && (__GNUC__==3) // gcc 3.1.1 at least
// #  define __FIT_TYPENAME_TEMPLATE_PARAMETER_RET
// #endif

#ifdef __FIT_EXPLICIT_BUG
#  define __FIT_EXPLICIT
#else // __FIT_EXPLICIT_BUG
#  define __FIT_EXPLICIT  explicit
#endif // __FIT_EXPLICIT_BUG

#ifdef __FIT_TYPENAME_TEMPLATE_PARAMETER_BUG
#  define __FIT_TYPENAME 
#else // __FIT_TYPENAME_TEMPLATE_PARAMETER_BUG
#  define __FIT_TYPENAME typename
#endif // __FIT_TYPENAME_TEMPLATE_PARAMETER_BUG

// #ifdef __FIT_TYPENAME_TEMPLATE_PARAMETER_RET
// #  define __FIT_TYPENAME_RET typename
// #else
// #  define __FIT_TYPENAME_RET
// #endif

#ifdef __FIT_NEED_TYPENAME_IN_ARGS_BUG
#  define __FIT_TYPENAME_ARG typename
#else // __FIT_NEED_TYPENAME_IN_ARGS_BUG
#  define __FIT_TYPENAME_ARG 
#endif // __FIT_NEED_TYPENAME_IN_ARGS_BUG

#ifdef WIN32
// #include <winsock.h>
// #  if defined(BIGENDIAN) && (BIGENDIAN > 0)
// #    define _BIG_ENDIAN
// #  else
#    define _LITTLE_ENDIAN
// #  endif
#else // !WIN32
#  ifdef __sun
#    include <sys/isa_defs.h>
#  elif defined(__hpux)
#    include <machine/param.h>
#  else // elif defined(__linux)
#    include <sys/types.h>
#    if !defined(__BYTE_ORDER) || !defined(__LITTLE_ENDIAN) || !defined(__BIG_ENDIAN)
#      error "One of __BYTE_ORDER, __LITTLE_ENDIAN and __BIG_ENDIAN undefined; Fix me!"
#    endif
#    if ( __BYTE_ORDER == __LITTLE_ENDIAN )
#      define _LITTLE_ENDIAN
#    elif ( __BYTE_ORDER == __BIG_ENDIAN )
#      define _BIG_ENDIAN
#    else
#      error "__BYTE_ORDER neither __BIG_ENDIAN nor __LITTLE_ENDIAN; Fix me!"
#    endif
#  endif
#endif // WIN32

#ifdef WIN32
#  define __FIT_SELECT // use select system call
#else // most current unixes has poll (HP-UX, old Linuxes may not)
#  if !defined(__FIT_SELECT)
#    define __FIT_POLL   // use poll system call
#  endif
#endif

#endif // __config_feature_h
