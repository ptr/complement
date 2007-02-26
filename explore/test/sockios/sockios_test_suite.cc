// -*- C++ -*- Time-stamp: <07/02/26 15:33:23 ptr>

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
  boost::shared_ptr<names_sockios_test> names_instance( new names_sockios_test() );

  test_case *hostname_tc = BOOST_CLASS_TEST_CASE( &names_sockios_test::hostname_test, names_instance );
  test_case *service_tc = BOOST_CLASS_TEST_CASE( &names_sockios_test::service_test, names_instance );

  test_case *hostaddr1_tc = BOOST_CLASS_TEST_CASE( &names_sockios_test::hostaddr_test1, names_instance );
  test_case *hostaddr2_tc = BOOST_CLASS_TEST_CASE( &names_sockios_test::hostaddr_test2, names_instance );
  test_case *hostaddr3_tc = BOOST_CLASS_TEST_CASE( &names_sockios_test::hostaddr_test3, names_instance );

  add( hostname_tc );
  add( service_tc );

  add( hostaddr1_tc );
  add( hostaddr2_tc );
  add( hostaddr3_tc );


  boost::shared_ptr<sockios_test> instance( new sockios_test() );
  test_case *init_tc = BOOST_CLASS_TEST_CASE( &sockios_test::init, instance );
  test_case *finit_tc = BOOST_CLASS_TEST_CASE( &sockios_test::finit, instance );

  test_case *ctor_dtor_tc = BOOST_CLASS_TEST_CASE( &sockios_test::ctor_dtor, instance );
  test_case *long_msg_tc  = BOOST_CLASS_TEST_CASE( &sockios_test::long_msg, instance );
  test_case *sigpipe_tc   = BOOST_CLASS_TEST_CASE( &sockios_test::sigpipe, instance );

  test_case *read0_tc     = BOOST_CLASS_TEST_CASE( &sockios_test::read0, instance );
  test_case *read0_srv_tc = BOOST_CLASS_TEST_CASE( &sockios_test::read0_srv, instance );
  test_case *long_block_read_tc = BOOST_CLASS_TEST_CASE( &sockios_test::long_block_read, instance );

  long_msg_tc->depends_on( init_tc );
  long_msg_tc->depends_on( ctor_dtor_tc );
  sigpipe_tc->depends_on( ctor_dtor_tc );
  sigpipe_tc->depends_on( init_tc );
  read0_tc->depends_on( sigpipe_tc );
  read0_srv_tc->depends_on( sigpipe_tc );
  long_block_read_tc->depends_on( init_tc );
  finit_tc->depends_on( init_tc );

  add( init_tc );
  add( ctor_dtor_tc );
  add( long_msg_tc );
  add( sigpipe_tc );
  add( read0_tc, 0, 5 );
  add( read0_srv_tc );
  add( long_block_read_tc );
  add( finit_tc );
}
