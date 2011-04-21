/* Time-stamp: <2011-03-23 17:31:37 ptr> */

/*
 * Copyright (c) 1999, 2002-2011
 * Petr Ovtchenkov
 *
 * Portion Copyright (c) 1999-2001
 * Parallel Graphics Ltd.
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#ifndef __config_feature_h
#define __config_feature_h

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
#  ifdef __INTEL_COMPILER
#    include <config/_icc.h>
#  endif
/* Intel's icc define __GNUC__! */
#  if defined(__GNUC__) && !defined(__INTEL_COMPILER)
#    include <config/_gcc.h>
#  endif
#endif

#ifdef __FreeBSD__
#  include <config/_freebsd.h>
#  ifdef __GNUC__
#    include <config/_gcc.h>
#  endif
#endif

#ifdef __OpenBSD__
#  include <config/_openbsd.h>
#  ifdef __GNUC__
#    include <config/_gcc.h>
#  endif
#endif

#ifdef WIN32
#include <config/_windows.h>
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

/*
 * Windows has no poll system call, while most current unixes
 * has poll (HP-UX 10.xx, old Linuxes may not).
 * We can use either poll or select.
 */
#if !defined(__FIT_NO_POLL) /* && !defined(__FIT_EPOLL) */
# define __FIT_POLL   /* use poll system call */
#endif

#ifdef __FIT_EXPLICIT_BUG
#  define __FIT_EXPLICIT
#else /* __FIT_EXPLICIT_BUG */
#  define __FIT_EXPLICIT  explicit
#endif /* __FIT_EXPLICIT_BUG */

#ifdef __FIT_TYPENAME_TEMPLATE_PARAMETER_BUG
#  define __FIT_TYPENAME 
#else /* __FIT_TYPENAME_TEMPLATE_PARAMETER_BUG */
#  define __FIT_TYPENAME typename
#endif /* __FIT_TYPENAME_TEMPLATE_PARAMETER_BUG */

/*
#ifdef __FIT_TYPENAME_TEMPLATE_PARAMETER_RET
#  define __FIT_TYPENAME_RET typename
#else
#  define __FIT_TYPENAME_RET
#endif
*/

#ifdef __FIT_NEED_TYPENAME_IN_ARGS_BUG
#  define __FIT_TYPENAME_ARG typename
#else /* __FIT_NEED_TYPENAME_IN_ARGS_BUG */
#  define __FIT_TYPENAME_ARG 
#endif /* __FIT_NEED_TYPENAME_IN_ARGS_BUG */

/*
 Store information about stack before create thread in std::tr2::thread;
 useful for debugging. Real implementation require BFD.
*/

/*
#define __FIT_CREATE_THREAD_STACK_INFO
*/

/*
 Don't use bfd, even if it available on platform; printing stack
 impossible without BFD (Binary File Descriptor).
*/

/* #define __FIT_DISABLE_BFD */

#ifdef __FIT_DISABLE_BFD
#  ifdef __FIT_PRESENT_BFD
#    undef __FIT_PRESENT_BFD
#  endif
/* Without BFD we can't take info about stack */
#if 0
#  ifdef __FIT_CREATE_THREAD_STACK_INFO
#    undef __FIT_CREATE_THREAD_STACK_INFO
#  endif
#endif
#endif

/*
 Constructions from C++ 0x draft
 */
#if defined(__GNUC__) && defined(__GXX_EXPERIMENTAL_CXX0X__)
#  define __FIT_CPP_0X
#endif

#endif /* __config_feature_h */
