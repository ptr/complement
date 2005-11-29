/* -*- C -*- Time-stamp: <05/09/02 00:27:37 ptr> */

/*
 *
 * Copyright (c) 2003
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

#ifndef __config__gcc_h
#define __config__gcc_h

/*
#if defined(__GNUC__) && (__GNUC__==3) // gcc 3.1.1 at least
#  define __FIT_TYPENAME_TEMPLATE_PARAMETER_RET
#endif
*/
#if defined(__GNUC__) && ( __GNUC__ == 2 ) && (__GNUC_MINOR__ == 95)
/* at least 2.95.3
#define __FIT_STD_AS_GLOBAL
*/
#endif

#endif /* __config__gcc_h */
