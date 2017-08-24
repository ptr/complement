// -*- C++ -*-

/*
 * Copyright (c) 2006-2009, 2017
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include "mt_test_suite.h"
#include "shm_test.h"
#include "mt_test_wg21.h"
#include "sys_err_test.h"
#include "flock_test.h"
#include "misc.h"

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
  Opts opts;

  opts.description( "test suite for 'xmt' framework" );
  opts.usage( "[options]" );

  opts << option<void>( "print this help message", 'h', "help" )
       << option<void>( "list all test cases", 'l', "list" )
       << option<std::string>( "run tests by number <num list>", 'r', "run" )
       << option<void>( "print status of tests within test suite", 'v', "verbose" )
       << option<void>(  "trace checks", 't', "trace" );

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

  exam::test_suite t( "libxmt test" );
  sys_err_test sys_err;

  t.add( signal_1_test, "signal_1_test" );
  t.add( signal_3_test, "signal_3_test" );

#if 0
  t.add( timespec_diff, "timespec_diff" );
  // You can't throw exception from signal handler
  // (stack saved/restored, that confuse stack unwind);
  // by this reason next test is commented:
  // t.add( signal_2_test, "signal_2_test" );
#endif

  exam::test_suite::test_case_type tc[3];

  t.add( &sys_err_test::file, sys_err, "system error, no such file" );

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
  t.add( &mt_test_wg21::mutex_rw_test, test_wg21, "rw mutex",
    t.add( &mt_test_wg21::mutex_test, test_wg21, "mt_test_wg21::mutex_test" ) );
  t.add( &mt_test_wg21::barrier, test_wg21, "mt_test_wg21::barrier" );
  t.add( &mt_test_wg21::semaphore, test_wg21, "mt_test_wg21::semaphore" );
  t.add( &mt_test_wg21::condition_var, test_wg21, "mt_test_wg21::condition_variable" );
  t.add( &mt_test_wg21::pid, test_wg21, "mt_test_wg21::pid",
         t.add( &mt_test_wg21::fork, test_wg21, "mt_test_wg21::fork" ) );

  uid_test_wg21 test_wg21_uid;

  exam::test_suite::test_case_type uidtc[6];

  uidtc[1] = t.add( &uid_test_wg21::uid, test_wg21_uid, "uid_test_wg21::uid",
         uidtc[0] = t.add( &uid_test_wg21::uidstr, test_wg21_uid, "uid_test_wg21::uidstr" ) );
  uidtc[2] = t.add( &uid_test_wg21::istream, test_wg21_uid, "istream get position issue" );
  uidtc[3] = t.add( &uid_test_wg21::uidconv, test_wg21_uid, "uid_test_wg21::uidconv",
                    t.add( &uid_test_wg21::hostidstr, test_wg21_uid, "uid_test_wg21::hostidstr",
                           t.add( &uid_test_wg21::hostid, test_wg21_uid, "uid_test_wg21::hostid" ) ) );
  t.add( &uid_test_wg21::version, test_wg21_uid, "uid version", uidtc[1] );
  uidtc[4] = t.add( &uid_test_wg21::copy_n, test_wg21_uid, "copy_n",
                    t.add( &uid_test_wg21::istream_iterator, test_wg21_uid, "istream_iterator",
                           t.add( &uid_test_wg21::istream_iterator_ctor, test_wg21_uid, "istream_iterator ctor", uidtc[2] ) ) );
  uidtc[5] = t.add( &uid_test_wg21::sentry, test_wg21_uid, "sentry" );
  t.add( &uid_test_wg21::uid_stream, test_wg21_uid, "uid_test_wg21::uid_stream", uidtc + 2, uidtc + 6 );

  flock_test flock;

  exam::test_suite::test_case_type flocktc[3];

  // t.add( &flock_test::write_profane, flock, "file lock, write profane",

  t.add( &flock_test::format, flock, "file lock, format io",
    flocktc[1] = t.add( &flock_test::read_lock, flock, "file lock, write",
      flocktc[0] = t.add( &flock_test::read_lock, flock, "file lock, read",
        t.add( &flock_test::create, flock, "create reference file", uidtc[0] ) ) ) );

  t.add( &flock_test::wr_lock, flock, "file lock, exclusive/shared", flocktc, flocktc + 2 );
  t.add( &flock_test::rw_lock, flock, "file lock, shared/exclusive", flocktc, flocktc + 2 );
  t.add( &flock_test::try_lock, flock, "try lock test", flocktc, flocktc + 2 );

  mt_test_misc misc;

  t.add( &mt_test_misc::demangle, misc, "demangle" );
  t.add( &mt_test_misc::function, misc, "function" );

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
}
