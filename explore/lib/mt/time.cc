// -*- C++ -*- Time-stamp: <02/07/14 13:53:45 ptr>

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

#pragma VERSIONID "@(#)$Id$"
#  ifdef __HP_aCC
#pragma ident "@(#)$Id$"
#  else
#ident "@(#)$Id$"
#  endif
#endif

#include <mt/time.h>

#ifdef _WIN32
#include <mt/xmt.h>
#ifdef __sun
std::string calendar_time( time_t t )
{
#ifdef __linux
  ctime_r( &t, buff );
#endif
#if !defined(__sun) && !defined(__linux)
#  error( "Fix me!" )
#endif
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
