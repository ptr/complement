// -*- C++ -*- Time-stamp: <08/07/30 20:51:24 ptr>

/*
 * Copyright (c) 2006-2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include "mt_test_wg21.h"

#include <mt/date_time>
#include <mt/thread>
#include <mt/mutex>
#include <mt/condition_variable>
#include <misc/type_traits.h>
#include <typeinfo>

#include <iostream>
#include <sstream>

#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <mt/uid.h>

#include <string>
#include <set>

int EXAM_IMPL(mt_test_wg21::date_time)
{
  // using namespace std::tr2;

  // check core of implementation
  EXAM_CHECK( (std::tr1::is_same<std::tr2::nanoseconds,std::tr2::detail::__is_more_precise<std::tr2::nanoseconds,std::tr2::microseconds>::finest_type>::value) );
  EXAM_CHECK( (std::tr1::is_same<std::tr2::nanoseconds,std::tr2::detail::__is_more_precise<std::tr2::microseconds,std::tr2::nanoseconds>::finest_type>::value) );

  // the same, just with typeid
  EXAM_CHECK( typeid(std::tr2::nanoseconds) == typeid(std::tr2::detail::__is_more_precise<std::tr2::nanoseconds,std::tr2::microseconds>::finest_type) );
  EXAM_CHECK( typeid(std::tr2::nanoseconds) == typeid(std::tr2::detail::__is_more_precise<std::tr2::microseconds,std::tr2::nanoseconds>::finest_type) );

  std::tr2::nanoseconds ns( 100 );
  std::tr2::nanoseconds ns2( 200 );

  EXAM_CHECK( ns.count() == 100 );
  EXAM_CHECK( ns < ns2 );

  std::tr2::microseconds mc( 100 );
  EXAM_CHECK( mc.count() == 100 );
  EXAM_CHECK( ns < mc );
  EXAM_CHECK( (ns * 1000L) == mc );
  EXAM_CHECK( (mc / 1000L) != ns ); // Before conversion: (mc / 1000L) == 0
  EXAM_CHECK( (mc + ns) > mc );
  EXAM_CHECK( (mc + ns).count() == (ns + mc).count() );
  EXAM_CHECK( (mc + ns) == (ns + mc) );

  std::tr2::nanoseconds ns3( 100000LL );
  EXAM_CHECK( ns3 == static_cast<std::tr2::nanoseconds>(mc) );
  EXAM_CHECK( ns3 == mc );
  EXAM_CHECK( mc == ns3 );

  std::tr2::system_time st = std::tr2::get_system_time();

  EXAM_CHECK( (std::tr2::get_system_time() - st) >= std::tr2::nanoseconds( 0 ) );

  return EXAM_RESULT;
}

static int val = 0;

void thread_func()
{
  val = 1;
}

void thread_func_int( int v )
{
  val = v;
}

int EXAM_IMPL(mt_test_wg21::thread_call)
{
  val = -1;
 
  std::tr2::basic_thread<0,0> t( thread_func );

  t.join();

  EXAM_CHECK( val == 1 );

  std::tr2::basic_thread<0,0> t2( thread_func_int, 2 );

  t2.join();

  EXAM_CHECK( val == 2 );

  val = 0;

  return EXAM_RESULT;
}

static std::tr2::mutex lk;

void thread_func2()
{
  std::tr2::lock_guard<std::tr2::mutex> lock( lk );

  ++val;
}

int EXAM_IMPL(mt_test_wg21::mutex_test)
{
  val = 0;

  std::tr2::basic_thread<0,0> t( thread_func2 );

  lk.lock();
  --val;
  lk.unlock();

  t.join();

  EXAM_CHECK( val == 0 );

  std::tr2::recursive_mutex rlk;

  rlk.lock();
  rlk.lock(); // shouldn't block here
  rlk.unlock();
  rlk.unlock();

  return EXAM_RESULT;
}

static std::tr2::barrier bar;

void thread_func3()
{
  try {
    EXAM_CHECK_ASYNC( val == 0 );

    bar.wait();

    std::tr2::lock_guard<std::tr2::mutex> lock( lk );

    ++val;
  }
  catch ( std::runtime_error& err ) {
    EXAM_ERROR_ASYNC( err.what() );
  }
}

int EXAM_IMPL(mt_test_wg21::barrier)
{
  val = 0;

  std::tr2::basic_thread<0,0> t( thread_func3 );

  EXAM_CHECK( val == 0 );

  bar.wait();

  lk.lock();
  --val;
  lk.unlock();

  t.join();

  EXAM_CHECK( val == 0 );

  return EXAM_RESULT;
}

void thread_func4(  std::tr2::semaphore* s )
{
  EXAM_CHECK_ASYNC( val == 1 );

  val = 0;

  s->notify_one();
}

int EXAM_IMPL(mt_test_wg21::semaphore)
{
  std::tr2::semaphore s;

  val = 1;

  s.wait();

  std::tr2::basic_thread<0,0> t( thread_func4, &s );

  s.wait();

  EXAM_CHECK( val == 0 );

  t.join();

  val = 1;

  std::tr2::semaphore s1(0);

  std::tr2::basic_thread<0,0> t1( thread_func4, &s1 );

  s1.wait();

  EXAM_CHECK( val == 0 );

  t1.join();

  // notify _before_ wait acceptable:

  s1.notify_one();
  s1.wait();

  return EXAM_RESULT;
}

int EXAM_IMPL(mt_test_wg21::fork)
{
  // trivial fork

  int v = 3;
  try {
    std::tr2::this_thread::fork();

    try {

      // Child code 
      EXAM_CHECK_ASYNC( v == 3 );

      v = 5;
    }
    catch ( ... ) {
    }

    exit( 0 );
  }
  catch ( std::tr2::fork_in_parent& child ) {
    try {
      EXAM_CHECK( child.pid() > 0 );

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }
      EXAM_CHECK( v == 3 );
    }
    catch ( ... ) {
    }
  }
  catch ( ... ) {
  }


  // less trivial fork: check interprocess communication via shared memory

  shmid_ds ds;
  int id = shmget( 5000, 1024, IPC_CREAT | IPC_EXCL | 0600 );
  EXAM_REQUIRE( id != -1 );
  // if ( id == -1 ) {
  //   cerr << "Error on shmget" << endl;
  // }
  EXAM_REQUIRE( shmctl( id, IPC_STAT, &ds ) != -1 );
  // if ( shmctl( id, IPC_STAT, &ds ) == -1 ) {
  //   cerr << "Error on shmctl" << endl;
  // }
  void *buf = shmat( id, 0, 0 );
  EXAM_REQUIRE( buf != reinterpret_cast<void *>(-1) );
  // if ( buf == reinterpret_cast<void *>(-1) ) {
  //   cerr << "Error on shmat" << endl;
  // }

  int& x = *new( buf ) int(4);

  EXAM_CHECK( x == 4 );

  try {
    std::tr2::this_thread::fork();

    try {

      // Child code 
      EXAM_CHECK_ASYNC( v == 3 );

      v = 5;

      EXAM_CHECK_ASYNC( x == 4 );

      x = 6;
    }
    catch ( ... ) {
    }

    exit( 0 );
  }
  catch ( std::tr2::fork_in_parent& child ) {
    try {
      EXAM_CHECK( child.pid() > 0 );

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }
      EXAM_CHECK( v == 3 );
      EXAM_CHECK( x == 6 );
    }
    catch ( ... ) {
    }
  }
  catch ( ... ) {
  }

  shmdt( buf );
  shmctl( id, IPC_RMID, &ds );

  return EXAM_RESULT;
}

int EXAM_IMPL(uid_test_wg21::uidstr)
{
  std::string u1 = xmt::uid_str();

  EXAM_CHECK( !u1.empty() );

  EXAM_CHECK( u1.length() == 36 );

  std::string u2 = xmt::uid_str();

  EXAM_CHECK( !u2.empty() );

  EXAM_CHECK( u2.length() == 36 );

  EXAM_CHECK( u1 != u2 );

  std::set<std::string> cnt;

  for ( int i = 0; i < 100; ++i ) {
    std::string s = xmt::uid_str();

    EXAM_REQUIRE( s.length() == 36 );
    EXAM_REQUIRE( s[8] == '-' );
    EXAM_REQUIRE( s[13] == '-' );
    EXAM_REQUIRE( s[18] == '-' );
    EXAM_REQUIRE( s[23] == '-' );

    EXAM_REQUIRE( s.find_first_not_of( "0123456789abcdef-" ) == std::string::npos );
    EXAM_CHECK( cnt.find( s ) == cnt.end() );
    cnt.insert( s );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(uid_test_wg21::hostidstr)
{
  std::string u1 = xmt::hostid_str();

  EXAM_CHECK( !u1.empty() );

  EXAM_CHECK( u1.length() == 36 );

  std::string u2 = xmt::hostid_str();

  EXAM_CHECK( !u2.empty() );

  EXAM_CHECK( u1 == u2 );

  return EXAM_RESULT;
}

int EXAM_IMPL(uid_test_wg21::hostid)
{
  xmt::uuid_type u1 = xmt::hostid();
  xmt::uuid_type u2 = xmt::hostid();

  EXAM_CHECK( u1 == u2 );

  return EXAM_RESULT;
}

int EXAM_IMPL(uid_test_wg21::uid)
{
  xmt::uuid_type u1 = xmt::uid();
  xmt::uuid_type u2 = xmt::uid();

  EXAM_CHECK( u1 != u2 );

  return EXAM_RESULT;
}

int EXAM_IMPL(uid_test_wg21::uidconv)
{
  xmt::uuid_type u1 = xmt::hostid();
  std::string u2 = xmt::hostid_str();

  EXAM_CHECK( static_cast<std::string>(u1) == u2 ); // <-- conversion to string

  std::stringstream s;

  s << u1;

  EXAM_CHECK( s.str() == u2 );

  return EXAM_RESULT;
}
