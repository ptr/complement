// -*- C++ -*- Time-stamp: <99/10/04 10:50:21 ptr>

/*
 *
 * Copyright (c) 1999
 * Petr Ovchenkov
 *
 * Copyright (c) 1999
 * ParallelGraphics Software Systems
 
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

#ident "$SunId$ %Q%"

#ifdef _DEBUG
#  ifndef __STL_DEBUG
#    define __STL_DEBUG
#  endif // __STL_DEBUG
#endif // _DEBUG

#ifdef _DEBUG_ALLOC
#  ifndef __STL_DEBUG_ALLOC
#    define __STL_DEBUG_ALLOC
#  endif // __STL_DEBUG_ALLOC
#endif // _DEBUG_ALLOC

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


#ifdef __SunOS_5_6
#  define _XOPEN_SOURCE
#  define _XOPEN_SOURCE_EXTENDED 1
#  define __EXTENSIONS__
#  define __XPG4v2
#endif // __SunOS_5_6

#ifdef __SunOS_5_7
#  define _XOPEN_SOURCE
#  define _XOPEN_SOURCE_EXTENDED 1
#  define __EXTENSIONS__
#  define __XPG4v2
#endif // __SunOS_5_7

#ifdef __SUNPRO_CC
#  ifndef __LINK_TIME_INSTANTIATION
#    define __LINK_TIME_INSTANTIATION
#  endif
// #  define __PG_INCLASS_OPERATOR
#endif // __SUNPRO_CC

#endif // __config_feature_h
