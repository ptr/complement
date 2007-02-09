// -*- C++ -*- Time-stamp: <07/02/09 16:00:57 ptr>

/*
 * Copyright (c) 2006, 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include "mt_test_suite.h"
#include "mt_test.h"

#include <config/feature.h>

using namespace boost::unit_test_framework;

mt_test_suite::mt_test_suite() :
    test_suite( "mt test suite" )
{
  boost::shared_ptr<mt_test> instance( new mt_test() );

  test_case *barrier_tc = BOOST_CLASS_TEST_CASE( &mt_test::barrier, instance );
  test_case *join_tc = BOOST_CLASS_TEST_CASE( &mt_test::join_test, instance );
  test_case *barrier2_tc = BOOST_CLASS_TEST_CASE( &mt_test::barrier2, instance );
  test_case *yield_tc = BOOST_CLASS_TEST_CASE( &mt_test::yield, instance );
  test_case *mutex_test_tc = BOOST_CLASS_TEST_CASE( &mt_test::mutex_test, instance );
#ifdef __FIT_PTHREAD_SPINLOCK
  test_case *spinlock_test_tc = BOOST_CLASS_TEST_CASE( &mt_test::spinlock_test, instance );
#endif
  test_case *recursive_mutex_test_tc = BOOST_CLASS_TEST_CASE( &mt_test::recursive_mutex_test, instance );
  test_case *fork_tc = BOOST_CLASS_TEST_CASE( &mt_test::fork, instance );
  test_case *pid_tc = BOOST_CLASS_TEST_CASE( &mt_test::pid, instance );
  test_case *shm_segment_tc = BOOST_CLASS_TEST_CASE( &mt_test::shm_segment, instance );
  test_case *shm_alloc_tc = BOOST_CLASS_TEST_CASE( &mt_test::shm_alloc, instance );
  test_case *fork_shm_tc = BOOST_CLASS_TEST_CASE( &mt_test::fork_shm, instance );
  test_case *shm_nm_obj_tc = BOOST_CLASS_TEST_CASE( &mt_test::shm_named_obj, instance );

  test_case *thr_mgr_tc = BOOST_CLASS_TEST_CASE( &mt_test::thr_mgr, instance );

  test_case *shm_init_tc = BOOST_CLASS_TEST_CASE( &mt_test::shm_init_mgr, instance );
  test_case *shm_finit_tc = BOOST_CLASS_TEST_CASE( &mt_test::shm_finit_mgr, instance );

  barrier2_tc->depends_on( barrier_tc );
  barrier2_tc->depends_on( join_tc );
  yield_tc->depends_on( barrier2_tc );
  mutex_test_tc->depends_on( yield_tc );
#ifdef __FIT_PTHREAD_SPINLOCK
  spinlock_test_tc->depends_on( yield_tc );
#endif
  recursive_mutex_test_tc->depends_on( mutex_test_tc );

  pid_tc->depends_on( fork_tc );
  shm_alloc_tc->depends_on( shm_segment_tc );
  fork_shm_tc->depends_on( shm_alloc_tc );
  shm_nm_obj_tc->depends_on( fork_shm_tc );

  shm_init_tc->depends_on( shm_alloc_tc );
  shm_finit_tc->depends_on( shm_init_tc );

  add( barrier_tc, 0, 2 );
  add( join_tc );
  add( barrier2_tc, 0, 3 );
  add( yield_tc, 0, 3 );
  add( mutex_test_tc, 0, 3 );
#ifdef __FIT_PTHREAD_SPINLOCK
  add( spinlock_test_tc, 0, 3 );
#endif
  add( recursive_mutex_test_tc, 0, 3 );

  add( fork_tc );
  add( pid_tc );
  add( shm_segment_tc );
  add( shm_alloc_tc );
  add( fork_shm_tc, 0, 5 );
  add( shm_nm_obj_tc, 0, 5 );

  add( thr_mgr_tc );

  add( shm_init_tc );
  add( shm_finit_tc );
};
