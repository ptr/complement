// -*- C++ -*- Time-stamp: <08/07/02 13:01:53 yeti>

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

#include <misc/opts.h>
#include <string>

int EXAM_DECL(timespec_diff);
int EXAM_DECL(signal_1_test);
// int EXAM_DECL(signal_2_test);
int EXAM_DECL(signal_3_test);

// flock requre revision, commented now.
// int EXAM_DECL( flock_test );
// int EXAM_DECL( lfs_test );

int main( int argc, const char** argv )
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

  uid_test_wg21 test_wg21_uid;

  t.add( &uid_test_wg21::uid, test_wg21_uid, "uid_test_wg21::uid",
         t.add( &uid_test_wg21::uidstr, test_wg21_uid, "uid_test_wg21::uidstr" ) );
  t.add( &uid_test_wg21::hostid, test_wg21_uid, "uid_test_wg21::hostid",
         t.add( &uid_test_wg21::hostidstr, test_wg21_uid, "uid_test_wg21::hostidstr" ) );

  Opts opts;

  opts.description( "test suite for 'sockios' framework" );
  opts.usage( "[options]" );

  opts << option<bool>( "print this help message", 'h', "help" )
       << option<bool>( "list all test cases", 'l', "list" )
       << option<std::string>( "run tests by number", 'r', "run" )["0"]
       << option<bool>( "print status of tests within test suite", 'v', "verbose" )
       << option<bool>(  "trace checks", 't', "trace" );

  try {
    opts.parse( argc, argv );
  }
  catch (...) {
    opts.help( std::cerr );
    return 1;
  }

  if ( opts.is_set( 'h' ) ) {
    opts.help( std::cerr );
    return 0;
  }

  if ( opts.is_set( 'l' ) ) {
    t.print_graph( std::cerr );
    return 0;
  }

  if ( opts.is_set( 'v' ) ) {
    t.flags( t.flags() | exam::base_logger::verbose );
  }

  if ( opts.is_set( 't' ) ) {
    t.flags( t.flags() | exam::base_logger::trace );
  }

  if ( opts.is_set( 'r' ) ) {
    std::stringstream ss( opts.get<std::string>( 'r' ) );
    int n;
    while ( ss >> n ) {
      t.single( n );
    }

    return 0;
  }

  return t.girdle();
};
