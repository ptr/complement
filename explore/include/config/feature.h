// -*- C++ -*- Time-stamp: <03/09/24 19:51:34 ptr>

/*
 *
 * Copyright (c) 1999, 2002, 2003
 * Petr Ovtchenkov
 *
 * Portion Copyright (c) 1999-2001
 * Parallel Graphics Ltd.
 *
 * Licensed under the Academic Free License Version 1.2
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

#ifndef __config_feature_h
#define __config_feature_h

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#ident "@(#)$Id$"
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

#ifdef __sun
#  include <config/_solaris.h>
#  ifdef __GNUC__
#    include <config/_gcc.h>
#  endif
#  ifdef __SUNPRO_CC
#    include <config/_sunprocc.h>
#  endif
#endif

#ifdef __hpux
#  include <config/_hpux.h>
#  ifdef __GNUC__
#    include <config/_gcc.h>
#  endif
#  ifdef __HP_aCC
#    include <config/_hpacc.h>
#  endif
#endif

#ifdef linux
#  include <config/_linux.h>
#  ifdef __GNUC__
#    include <config/_gcc.h>
#  endif
#endif

#ifdef __FreeBSD__
#  include <config/_freebsd.h>
#  ifdef __GNUC__
#    include <config/_gcc.h>
#  endif
#endif

#ifdef WIN32
#include <config/_windows.h>
#endif

#ifdef N_PLAT_NLM
#include <config/_mwccnlm.h>
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

// Windows has no poll system call, while most current unixes
// has poll (HP-UX 10.xx, old Linuxes may not).
// We can use either poll or select.
#ifndef __FIT_NO_POLL
# define __FIT_POLL   // use poll system call
#endif

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

#endif // __config_feature_h
