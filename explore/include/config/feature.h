// -*- C++ -*- Time-stamp: <00/08/02 17:13:13 ptr>

/*
 *
 * Copyright (c) 1999
 * Petr Ovchenkov
 *
 * Copyright (c) 1999-2000
 * ParallelGraphics
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

#ident "$SunId$"

// #ifdef _DEBUG
// #  ifndef __STL_DEBUG
// #    define __STL_DEBUG  1
// #  endif // __STL_DEBUG
// #endif // _DEBUG

// #ifdef _DEBUG_ALLOC
// #  ifndef __STL_DEBUG_ALLOC
// #    define __STL_DEBUG_ALLOC  1
// #  endif // __STL_DEBUG_ALLOC
// #endif // _DEBUG_ALLOC

#define __USE_SGI_STL_PORT  0x400
// #define __USE_SGI_STL_PORT  0x321

#if /* defined( __STL_DEBUG ) && */ (__USE_SGI_STL_PORT >= 0x400)
// #  define __STD __stl_native_std
#  define __STD __STLPORT_STD // __std_alias
#  ifdef __STL_DEBUG 
#    define __VENDOR_STD  __STL_VENDOR_EXCEPT_STD
#  else
#    define __VENDOR_STD  __std_alias
#  endif
// #else
// #  ifndef _MSC_VER
// #    define __STD std
// #  else
// #    define __STD _STL
// #  endif 
#endif

#if 0
#if __USE_SGI_STL_PORT >= 0x322
// #  if __USE_SGI_STL_PORT < 0x400
#    define __STL_STD_REBUILD 1
#    define  __STL_USE_SGI_STRING  1
// # define   __STL_USE_NEW_IOSTREAMS	1
#    define __SGI_STL_OWN_IOSTREAMS 1
// # define  __STL_HAS_WCHAR_T
#    if !defined( _MSC_VER ) && defined(__STL_DEBUG)
#       define __STL_NO_OWN_NAMESPACE  1
// #      define __STL_DONT_REDEFINE_STD
#    elif !defined( _MSC_VER )
#      define __STL_DONT_REDEFINE_STD
#    endif
// #  define __STL_VENDOR_CSTD std
#    ifndef __GNUC__
#      define __STL_USE_NEW_C_HEADERS
#    else
#      define __STL_NO_CSTD_FUNCTION_IMPORTS
// #  define __STL_NO_METHOD_SPECIALIZATION
#    endif
// #  endif // __USE_SGI_STL_PORT

#  ifndef _REENTRANT
#    define _REENTRANT
#    ifdef __sun
#      define __STL_SOLARIS_THREADS
#    endif
#  else
#    ifdef __sun
#      define __STL_SOLARIS_THREADS
#    endif
#  endif
#  ifdef _MSC_VER
// #    define __STL_NO_STATIC_TEMPLATE_DATA
#  endif
#elif (__USE_SGI_STL_PORT == 0x321) && defined(_MSC_VER)
#  define __STL_USE_NEW_IOSTREAMS
#  define __STLPORT_NEW_IOSTREAMS 
#endif
#else

#  ifndef _REENTRANT
#    define _REENTRANT
#  endif
#  ifdef __sun
#    define __STL_SOLARIS_THREADS
#  endif

#endif // 0

#if defined( WIN32 ) && !defined(__PG_USE_STATIC_LIB) && !defined(_LIB)
#  undef  __PG_USE_DECLSPEC
#  define __PG_USE_DECLSPEC
#else
#  undef  __PG_USE_DECLSPEC
#endif

#if (defined (__DLL) || defined (_DLL) || defined (_WINDLL) )
#  undef  __PG_DLL
#  define __PG_DLL
#else 
#  undef  __PG_DLL
#endif

#ifdef __PG_USE_DECLSPEC /* using export/import technique */
#  ifdef __PG_DLL
#    define __PG_DECLSPEC   __declspec( dllexport )
#  else
#    define __PG_DECLSPEC   __declspec( dllimport )
#  endif
#else
#  define __PG_DECLSPEC
#endif

#if defined(__sun) && defined(__GNUC__)
#  define __PG_USE_ABBREV
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
#endif

// #ifdef // __SunOS_5_7
// #  define _XOPEN_SOURCE 1
// #  define _XOPEN_SOURCE_EXTENDED 1
// #  define __EXTENSIONS__ 1
// #  define __XPG4v2 1
// #endif // __SunOS_5_7

#ifdef __SUNPRO_CC
#  ifndef __LINK_TIME_INSTANTIATION
#    define __LINK_TIME_INSTANTIATION 1
#  endif
#  define __PG_INCLASS_OPERATOR
#endif // __SUNPRO_CC

#endif // __config_feature_h
