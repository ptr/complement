// -*- C++ -*- Time-stamp: <06/12/18 19:58:56 ptr>

/*
 * Copyright (c) 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include "mt_test_suite.h"
#include "mt_test.h"

using namespace boost::unit_test_framework;

mt_test_suite::mt_test_suite() :
    test_suite( "mt test suite" )
{
  boost::shared_ptr<mt_test> instance( new mt_test() );

  test_case *fork_tc = BOOST_CLASS_TEST_CASE( &mt_test::fork, instance );
  test_case *pid_tc = BOOST_CLASS_TEST_CASE( &mt_test::pid, instance );

  pid_tc->depends_on( fork_tc );

  add( fork_tc );
  add( pid_tc );
};
