// -*- C++ -*- Time-stamp: <07/01/30 10:57:38 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2005, 2006, 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "sockios_test_suite.h"
#include "sockios_test.h"

#include <boost/test/unit_test.hpp>

using namespace boost::unit_test_framework;


sockios_test_suite::sockios_test_suite() :
    test_suite( "sockios library test suite" )
{
  boost::shared_ptr<sockios_test> instance( new sockios_test() );
  test_case *hostname_tc = BOOST_CLASS_TEST_CASE( &sockios_test::hostname_test, instance );
  test_case *service_tc = BOOST_CLASS_TEST_CASE( &sockios_test::service_test, instance );

  test_case *hostaddr1_tc = BOOST_CLASS_TEST_CASE( &sockios_test::hostaddr_test1, instance );
  test_case *hostaddr2_tc = BOOST_CLASS_TEST_CASE( &sockios_test::hostaddr_test2, instance );
  test_case *hostaddr3_tc = BOOST_CLASS_TEST_CASE( &sockios_test::hostaddr_test3, instance );
  test_case *ctor_dtor_tc = BOOST_CLASS_TEST_CASE( &sockios_test::ctor_dtor, instance );
  test_case *sigpipe_tc   = BOOST_CLASS_TEST_CASE( &sockios_test::sigpipe, instance );
  test_case *long_msg_tc  = BOOST_CLASS_TEST_CASE( &sockios_test::long_msg_test, instance );

  // hostaddr2_tc->depends_on( hostaddr1_tc );

  add( hostname_tc );
  add( service_tc );

  add( hostaddr1_tc );
  add( hostaddr2_tc );
  add( hostaddr3_tc );
  add( ctor_dtor_tc );
  add( sigpipe_tc );
  add( long_msg_tc );
}
