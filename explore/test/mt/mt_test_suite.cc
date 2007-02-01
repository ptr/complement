// -*- C++ -*- Time-stamp: <07/01/31 23:51:46 ptr>

/*
 * Copyright (c) 2006, 2007
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
  test_case *shm_segment_tc = BOOST_CLASS_TEST_CASE( &mt_test::shm_segment, instance );
  test_case *shm_alloc_tc = BOOST_CLASS_TEST_CASE( &mt_test::shm_alloc, instance );
  test_case *fork_shm_tc = BOOST_CLASS_TEST_CASE( &mt_test::fork_shm, instance );
  test_case *shm_nm_obj_tc = BOOST_CLASS_TEST_CASE( &mt_test::shm_named_obj, instance );

  pid_tc->depends_on( fork_tc );
  shm_alloc_tc->depends_on( shm_segment_tc );
  fork_shm_tc->depends_on( shm_alloc_tc );
  shm_nm_obj_tc->depends_on( fork_shm_tc );

  add( fork_tc );
  add( pid_tc );
  add( shm_segment_tc );
  add( shm_alloc_tc );
  add( fork_shm_tc, 0, 5 );
  add( shm_nm_obj_tc, 0, 5 );
};
