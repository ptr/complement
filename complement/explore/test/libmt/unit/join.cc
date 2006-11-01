// -*- C++ -*- Time-stamp: <06/08/04 11:09:21 ptr>

/*
 * Copyright (c) 2003, 2006
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
 */

#include <boost/test/unit_test.hpp>

using namespace boost::unit_test_framework;

#include <mt/xmt.h>

using namespace xmt;

static int x = 0;

Thread::ret_code thread_entry_call( void * )
{
  x = 1;

  Thread::ret_code rt;
  rt.iword = 0;

  return rt;
}

void join_test()
{
  BOOST_CHECK( x == 0 );

  Thread t( thread_entry_call );

  t.join();

  BOOST_CHECK( x == 1 );
}
