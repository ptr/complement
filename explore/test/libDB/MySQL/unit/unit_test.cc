// -*- C++ -*- Time-stamp: <06/06/10 14:54:22 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2004
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

#include <boost/test/unit_test.hpp>
#include <config/feature.h>

using namespace boost::unit_test_framework;

void mysql_create_table();

test_suite *init_unit_test_suite( int argc, char * * const argv )
{
  test_suite *ts = BOOST_TEST_SUITE( "libDB test" );

  ts->add( BOOST_TEST_CASE( &mysql_create_table ) );

  return ts;
}
