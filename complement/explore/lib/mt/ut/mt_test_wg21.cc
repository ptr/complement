// -*- C++ -*- Time-stamp: <08/02/25 13:01:59 ptr>

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

#include <iostream>

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
  EXAM_CHECK_ASYNC( val == 0 );

  bar.wait();

  std::tr2::lock_guard<std::tr2::mutex> lock( lk );

  ++val;
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
