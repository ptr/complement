// -*- C++ -*- Time-stamp: <03/09/24 21:04:53 ptr>

/*
 *
 * Copyright (c) 2002, 2003
 * Petr Ovtchenkov
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
 */

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#ident "@(#)$Id$"
#  endif
#endif

#include <mt/time.h>

#ifdef _WIN32
#include <mt/xmt.h>

__impl::Mutex _l;
#endif

std::string calendar_time( time_t t )
{
#ifndef _WIN32
  char buff[32];
// #ifdef __sun
#if (_POSIX_C_SOURCE - 0 >= 199506L) || defined(_POSIX_PTHREAD_SEMANTICS) || defined(N_PLAT_NLM) || \
    defined(__FreeBSD__) || defined(__OpenBSD__)
  ctime_r( &t, buff );
#else
  ctime_r( &t, buff, 32 );
#endif
// #endif
// #ifdef __linux
//  ctime_r( &t, buff );
// #endif
// #if !defined(__sun) && !defined(__linux)
// #  error( "Fix me!" )
// #endif

  // buff[24] = 0; // null terminate
  std::string s;

  s.assign( buff, 24 );

  return s;
#else // _WIN32
  MT_REENTRANT( _l, _1 );
  return std::string( ctime( &t ) );
#endif
}
