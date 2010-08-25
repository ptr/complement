// -*- C++ -*- Time-stamp: <10/08/25 18:25:54 ptr>

/*
 * Copyright (c) 2003, 2006, 2007, 2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include <exam/suite.h>

#include <mt/thread>
#include <mt/condition_variable>

/* 
 * thread 2:  v = 1; create thread 1 ----------------------------------- join; v == 4?
 *                      \                                          /  
 * thread 1:             set handler; v == 1? - kill ----------- exit 
 *                                                 \                
 * handler (within thread 1):                         v == 1?; v = 4
 */

static void thread_one();
static void thread_two();

static std::tr2::thread* th_one = 0;
static std::tr2::thread* th_two = 0;

static int v = 0;

struct true_val
{
    bool operator()() const
      { return (v == 2); }
};

static std::tr2::mutex cond_mtx;
static std::tr2::condition_variable cnd;

extern "C" {
  static void handler( int );

  void handler( int )
  {
    v = 4;
    /*
     Note: you have very restricted list of system calls that you can use here
     (in the handler of signal) safely. In particular, you can't call pthread_*
     functions. Reason: async-signal-safe calls, Unix 98, POSIX 1002.1
     */
    // cerr << "thread_one: Handler" << endl;
    // Thread::signal_exit( SIGTERM );
    // send signal to caller thread to exit:
    // th_one->kill( SIGTERM );

    // v = 3; // not reached
  }
}

void thread_one()
{
  std::tr2::this_thread::unblock_signal( SIGINT ); // we wait this signal
  // Default handler make exit() call:
  //   Thread::signal_handler( SIGINT, SIG_DFL );
  // That's why I set own handler:
  std::tr2::this_thread::signal_handler( SIGINT, handler );

  {
    std::tr2::unique_lock<std::tr2::mutex> lk( cond_mtx );

    cnd.wait( lk, true_val() );
  }

  pthread_kill( th_one->native_handle(), SIGINT ); // send signal SIGINT to self
}

void thread_two()
{
  {
    std::tr2::unique_lock<std::tr2::mutex> lk( cond_mtx );
    v = 1;
  }

  std::tr2::thread t( thread_one ); // start thread_one
  th_one = &t;            // store address to be called from thread_one

  {
    std::tr2::unique_lock<std::tr2::mutex> lk( cond_mtx );
    v = 2;
    cnd.notify_one();
  }

  t.join();

  EXAM_CHECK_ASYNC( v == 4 );
}

int EXAM_IMPL(signal_1_test)
{
  std::tr2::thread t( thread_two );

  t.join();

  EXAM_CHECK( v == 4 );

  return EXAM_RESULT;
}
