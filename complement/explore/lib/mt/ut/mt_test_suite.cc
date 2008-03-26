// -*- C++ -*- Time-stamp: <08/03/26 10:12:21 ptr>

/*
 * Copyright (c) 2006-2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include "mt_test_suite.h"
#include "mt_test.h"
#include "shm_test.h"
#include "mt_test_wg21.h"

#include <config/feature.h>

int EXAM_DECL(timespec_diff);
int EXAM_DECL(signal_1_test);
// int EXAM_DECL(signal_2_test);
int EXAM_DECL(signal_3_test);

// flock requre revision, commented now.
// int EXAM_DECL( flock_test );
// int EXAM_DECL( lfs_test );

int EXAM_IMPL(mt_test_suite)
{
  exam::test_suite t( "libxmt test" );
  mt_test test;

#if 0
  t.add( timespec_diff, "timespec_diff" );
  t.add( signal_1_test, "signal_1_test" );
  // You can't throw exception from signal handler
  // (stack saved/restored, that confuse stack unwind);
  // by this reason next test is commented:
  // t.add( signal_2_test, "signal_2_test" );
  t.add( signal_3_test, "signal_3_test" );
#endif

  exam::test_suite::test_case_type tc[3];

  // t.add( &mt_test::callstack, test, "callstack" );
  tc[0] = t.add( &mt_test::barrier, test, "mt_test::barrier" );
  tc[1] = t.add( &mt_test::join_test, test, "mt_test::join_test" );
  tc[2] = t.add( &mt_test::yield, test, "mt_test::yield",
                 t.add( &mt_test::barrier2, test, "mt_test::barrier2",
                        tc, tc + 2 ) );
  t.add( &mt_test::recursive_mutex_test, test, "mt_test::recursive_mutex_test",
         t.add( &mt_test::mutex_test, test, "mt_test::mutex_test", tc[2] ) );

#ifdef __FIT_PTHREAD_SPINLOCK
  t.add( &mt_test::spinlock_test, test, "mt_test::spinlock_test", tc[2] );
#endif
  t.add( &mt_test::pid, test, "mt_test::pid",
         t.add( &mt_test::fork, test, "mt_test::fork" ) );

  t.add( &mt_test::thr_mgr, test, "mt_test::thr_mgr", tc[1] );

  shm_test shmtest;

  t.add( &shm_test::shm_named_obj_more, shmtest, "mt_test::shm_named_obj_more",
         t.add( &shm_test::shm_named_obj, shmtest, "mt_test::shm_named_obj",
                t.add( &shm_test::fork_shm, shmtest, "mt_test::fork_shm",
                       t.add( &shm_test::shm_alloc, shmtest, "mt_test::shm_alloc",
                              t.add( &shm_test::shm_segment, shmtest, "mt_test::shm_segment" ) ) ) )
 );

  mt_test_wg21 test_wg21;

  t.add( &mt_test_wg21::date_time, test_wg21, "mt_test_wg21::date_time" );
  t.add( &mt_test_wg21::thread_call, test_wg21, "mt_test_wg21::thread_call" );
  t.add( &mt_test_wg21::mutex_test, test_wg21, "mt_test_wg21::mutex_test" );
  t.add( &mt_test_wg21::barrier, test_wg21, "mt_test_wg21::barrier" );
  t.add( &mt_test_wg21::semaphore, test_wg21, "mt_test_wg21::semaphore" );
  t.add( &mt_test_wg21::fork, test_wg21, "mt_test_wg21::fork" );

  return t.girdle();
};
