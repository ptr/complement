// -*- C++ -*- Time-stamp: <04/05/06 18:38:06 ptr>

/*
 * Copyright (c) 2004
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 2.0
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

#include <mt/xmt.h>

#include <boost/test/unit_test.hpp>

using namespace boost::unit_test_framework;

void timespec_diff()
{
  timespec t1;
  t1.tv_sec = 1083852875;
  t1.tv_nsec = 16629000;

  timespec t2;
  t2.tv_sec = 1083852871;
  t2.tv_nsec = 365378000;

  timespec t3 = t1 - t2;

  BOOST_CHECK( t3.tv_sec == 3 );
  BOOST_CHECK( t3.tv_nsec == 651251000 );

  t3 = t1;
  t3 -= t2;

  BOOST_CHECK( t3.tv_sec == 3 );
  BOOST_CHECK( t3.tv_nsec == 651251000 );

  t1.tv_sec = 1;
  t1.tv_nsec = 1;

  t2.tv_sec = 0;
  t2.tv_nsec = 1;

  t3 = t1 - t2;

  BOOST_CHECK( t3.tv_sec == 1 );
  BOOST_CHECK( t3.tv_nsec == 0 );

  t3 = t1;
  t3 -= t2;
  
  BOOST_CHECK( t3.tv_sec == 1 );
  BOOST_CHECK( t3.tv_nsec == 0 );
}
