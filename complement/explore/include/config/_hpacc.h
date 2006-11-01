// -*- C++ -*- Time-stamp: <03/01/19 13:22:56 ptr>

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

#ifndef __config__hpacc_h
#define __config__hpacc_h

#pragma VERSIONID "@(#)$Id$"

#if (__HP_aCC==1) // aCC A.03.13
#  define __FIT_TEMPLATE_FORWARD_BUG
#  define __FIT_EXPLICIT_BUG
#  define __FIT_USING_NAMESPACE_BUG
// #  define __FIT_TYPENAME_TEMPLATE_PARAMETER_BUG
#  define __FIT_TYPENAME_TEMPLATE_PARAMETER
#  define __FIT_NEED_TYPENAME_IN_ARGS_BUG
#  define __FIT_TEMPLATE_TYPEDEF_BUG
#  define __FIT_TEMPLATE_CLASSTYPE_BUG
#endif

#endif // __config__hpacc_h
