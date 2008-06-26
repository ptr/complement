// -*- C++ -*- Time-stamp: <08/03/26 10:09:40 ptr>

/*
 * Copyright (c) 2006, 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include "mt_test.h"

#include <mt/xmt.h>
#include <mt/shm.h>
#include <mt/thr_mgr.h>
#include <mt/callstack.h>

#include <sys/shm.h>
#include <sys/wait.h>

#include <signal.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <iostream>

using namespace std;
namespace fs = boost::filesystem;

int EXAM_IMPL(mt_test::callstack)
{
  xmt::callstack( cerr );

  return EXAM_RESULT;
}

/* ******************************************************
 * Degenerate case: check that one thread pass throw
 * own barrier.
 */
int EXAM_IMPL(mt_test::barrier)
{
  xmt::barrier b( 1 );

  b.wait();

  return EXAM_RESULT;
}

/* ******************************************************
 * Start thread, join it. Check return value.
 */

static int x = 0;

xmt::Thread::ret_t thread_entry_call( void *p )
{
  xmt::barrier& b = *reinterpret_cast<xmt::barrier *>(p);
  b.wait();

  x = 1;

  // cerr << "XXX" << endl;

  return reinterpret_cast<xmt::Thread::ret_t>(2);
}

int EXAM_IMPL(mt_test::join_test)
{
  EXAM_CHECK( x == 0 );

  xmt::barrier b;

  xmt::Thread t( thread_entry_call, &b );

  // cerr << t.good() << " " << t.is_join_req() << endl;

  // void *r = t.join();

  // cerr << r << endl;
  b.wait();

  EXAM_CHECK( reinterpret_cast<int>( t.join() ) == 2 );

  EXAM_CHECK( x == 1 );

  return EXAM_RESULT;
}

/* ******************************************************
 * Start two threads, align ones on barrier, join.
 */

xmt::Thread::ret_t thread2_entry_call( void *p )
{
  xmt::barrier& b = *reinterpret_cast<xmt::barrier *>(p);
  b.wait();

  return reinterpret_cast<xmt::Thread::ret_t>(1);
}

int EXAM_IMPL(mt_test::barrier2)
{
  xmt::barrier b;

  xmt::Thread t1( thread2_entry_call, &b );
  // xmt::Thread t2( thread2_entry_call, &b );

  // EXAM_CHECK( reinterpret_cast<int>(t2.join()) == 1 );
  // std::cerr << t2.join() << std::endl;
  b.wait();

  EXAM_CHECK( reinterpret_cast<int>(t1.join()) == 1 );

  return EXAM_RESULT;
}

/* ******************************************************
 * Start two threads, align ones on barrier; one thread
 * relinquish control to other; join (within Thread dtors)
 */

xmt::Thread::ret_t thread3_entry_call( void *p )
{
  int flag = 0;
  xmt::barrier& b = *reinterpret_cast<xmt::barrier *>(p);
  b.wait();
  EXAM_CHECK_ASYNC_F( xmt::Thread::yield() == 0, flag );

  return reinterpret_cast<xmt::Thread::ret_t>(flag);
}

int EXAM_IMPL(mt_test::yield)
{
  {
    xmt::barrier b;

    // xmt::Thread t1( thread2_entry_call, &b );
    xmt::Thread t2( thread3_entry_call, &b );
    // .join()'s are in Thread's destructors
    b.wait();
  }

  return EXAM_RESULT;
}

/* ******************************************************
 * Test for plain mutex.
 *
 * Start two threads, align ones on barrier, thr2 relinquish
 * control to thr1;
 * thr1 acquire lock, try to yield control to thr2 (that 
 * should be blocked on m1);
 * Correct order checked by values of x.
 */

static xmt::mutex m1;

xmt::Thread::ret_t thr1( void *p )
{
  int flag = 0;

  xmt::barrier& b = *reinterpret_cast<xmt::barrier *>(p);
  b.wait();

  m1.lock();
  EXAM_CHECK_ASYNC_F( x == 0, flag );

  xmt::Thread::yield();

  EXAM_CHECK_ASYNC_F( x == 0, flag );
  x = 1;

  m1.unlock();

  return reinterpret_cast<xmt::Thread::ret_t>(flag);
}

xmt::Thread::ret_t thr2( void *p )
{
  int flag = 0;
  xmt::barrier& b = *reinterpret_cast<xmt::barrier *>(p);
  b.wait();
  for ( int i = 0; i < 128; ++i ) {
    xmt::Thread::yield();
  }

  m1.lock();
  EXAM_CHECK_ASYNC_F( x == 1, flag );
  x = 2;
  m1.unlock();

  return reinterpret_cast<xmt::Thread::ret_t>(flag);
}

int EXAM_IMPL(mt_test::mutex_test)
{
  x = 0;
  xmt::barrier b;

  xmt::Thread t1( thr1, &b );
  xmt::Thread t2( thr2, &b );

  EXAM_CHECK( t1.join() == 0 );
  EXAM_CHECK( t2.join() == 0 );

  EXAM_CHECK( x == 2 );

  return EXAM_RESULT;
}

/* ******************************************************
 * Test for spinlocks.
 *
 * Start two threads, align ones on barrier, thr2 relinquish
 * control to thr1;
 * thr1 acquire lock, try to yield control to thr2 (that 
 * should be blocked on sl1);
 * Correct order checked by values of x.
 */

#ifdef __FIT_PTHREAD_SPINLOCK
static xmt::spinlock sl1;

xmt::Thread::ret_t thr1s( void *p )
{
  int flag = 0;
  xmt::barrier& b = *reinterpret_cast<xmt::barrier *>(p);
  b.wait();

  sl1.lock();
  EXAM_CHECK_ASYNC_F( x == 0, flag );

  xmt::Thread::yield();

  EXAM_CHECK_ASYNC_F( x == 0, flag );
  x = 1;

  sl1.unlock();

  return reinterpret_cast<xmt::Thread::ret_t>(flag);
}

xmt::Thread::ret_t thr2s( void *p )
{
  int flag = 0;
  xmt::barrier& b = *reinterpret_cast<xmt::barrier *>(p);
  b.wait();
  for ( int i = 0; i < 128; ++i ) {
    xmt::Thread::yield();
  }

  sl1.lock();
  EXAM_CHECK_ASYNC_F( x == 1, flag );
  x = 2;
  sl1.unlock();

  return reinterpret_cast<xmt::Thread::ret_t>(flag);
}

#endif

int EXAM_IMPL(mt_test::spinlock_test)
{
#ifdef __FIT_PTHREAD_SPINLOCK
  x = 0;
  xmt::barrier b;

  xmt::Thread t1( thr1s, &b );
  xmt::Thread t2( thr2s, &b );

  EXAM_CHECK( t1.join() == 0 );
  EXAM_CHECK( t2.join() == 0 );

  EXAM_CHECK( x == 2 );
#endif
  return EXAM_RESULT;
}

/* ****************************************************** */

/*
 * Test for recursive-safe mutexes (detect deadlock)
 *
 * 1. Start thread 1. Acquire lock on m2.
 *
 * 2. Start thread 2.  Yield control to thread 1, to be sure that thread 1
 *    acquire lock on m2 first. Acquire lock on m2.
 *    Due to m2 locked in thread 1, waiting on m2.lock.
 *
 * 3. Thread 1 relinquish control to thread 2, to give it chance.
 *
 * 4. From thread 1 call function 'recursive', where acquire lock on m2,
 *    relinquish control to thread2, and then release m2.
 *    If mutex recursive-safe, function 'recursive' will finished correctly.
 *    Otherwise, deedlock will happen on m2.lock in 'recursive'.
 *
 * 5. Release m2 in thread 1.
 *
 * 6. Pass through m2 lock in thread 2. Call function 'recursive',
 *    where acquire lock on m2, relinquish control, and then release m2. See item 4 before.
 *
 * 7. Release m2 in thread 2.
 *
 * 8. Test finished.
 * 
 */

xmt::__mutex<true,false> m2;

void recursive()
{
  m2.lock();

  x = 2;
  xmt::Thread::yield();
  EXAM_CHECK_ASYNC( x == 2 );

  m2.unlock();
}

xmt::Thread::ret_t thr1r( void *p )
{
  int flag = 0;
  xmt::barrier& b = *reinterpret_cast<xmt::barrier *>(p);
  b.wait();

  m2.lock();

  EXAM_CHECK_ASYNC_F( x == 0, flag );
  x = 1;
  xmt::Thread::yield();
  EXAM_CHECK_ASYNC_F( x == 1, flag );
  recursive();
  EXAM_CHECK_ASYNC_F( x == 2, flag );
  x = 3;

  m2.unlock();

  return reinterpret_cast<xmt::Thread::ret_t>(flag);
}

xmt::Thread::ret_t thr2r( void *p )
{
  int flag = 0;
  xmt::barrier& b = *reinterpret_cast<xmt::barrier *>(p);
  b.wait();

  for ( int i = 0; i < 128; ++i ) {
    xmt::Thread::yield();
  }

  m2.lock();

  EXAM_CHECK_ASYNC_F( x == 3, flag );
  xmt::Thread::yield();
  recursive();  
  EXAM_CHECK_ASYNC_F( x == 2, flag );

  m2.unlock();

  return reinterpret_cast<xmt::Thread::ret_t>(flag);
}

int EXAM_IMPL(mt_test::recursive_mutex_test)
{
  x = 0;
  xmt::barrier b;

  xmt::Thread t1( thr1r, &b );
  xmt::Thread t2( thr2r, &b );

  EXAM_CHECK( t1.join() == 0 );
  EXAM_CHECK( t2.join() == 0 );

  EXAM_CHECK( x == 2 );

  return EXAM_RESULT;
}

/* ****************************************************** */

int EXAM_IMPL(mt_test::fork)
{
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

  xmt::__condition<true>& fcnd = *new( buf ) xmt::__condition<true>();
  fcnd.set( false );

  try {
    xmt::fork();

    try {

      // Child code 

    }
    catch ( ... ) {
    }

    exit( 0 );
  }
  catch ( xmt::fork_in_parent& child ) {
    try {
      EXAM_CHECK( child.pid() > 0 );

      fcnd.set( true );

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }
    }
    catch ( ... ) {
    }
  }
  catch ( ... ) {
  }

  (&fcnd)->~__condition<true>();

  shmdt( buf );
  shmctl( id, IPC_RMID, &ds );

  return EXAM_RESULT;
}

/* ****************************************************** */

int EXAM_IMPL(mt_test::pid)
{
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

  xmt::__condition<true>& fcnd = *new( buf ) xmt::__condition<true>();
  fcnd.set( false );
  pid_t my_pid = xmt::getpid();

  try {
    xmt::fork();

    int flag = 0;

    try {

      // Child code 
      EXAM_CHECK_ASYNC_F( my_pid == xmt::getppid(), flag );
      *reinterpret_cast<pid_t *>(static_cast<char *>(buf) + sizeof(xmt::__condition<true>)) = xmt::getpid();

      fcnd.set( true );

    }
    catch ( ... ) {
    }

    exit( flag );
  }
  catch ( xmt::fork_in_parent& child ) {
    try {
      EXAM_CHECK( child.pid() > 0 );

      fcnd.try_wait();

      EXAM_CHECK( *reinterpret_cast<pid_t *>(static_cast<char *>(buf) + sizeof(xmt::__condition<true>)) == child.pid() );

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }
    }
    catch ( ... ) {
    }
  }
  catch ( ... ) {
  }

  (&fcnd)->~__condition<true>();

  shmdt( buf );
  shmctl( id, IPC_RMID, &ds );

  return EXAM_RESULT;
}

/* ******************************************************
 * Thread pool (aka ThreadMgr) test.
 * 
 * Start 200 threads under ThreadMgr; check that all threads
 * started, check that all theads finished, no garbage in
 * ThreadMgr remains.
 */

static int my_thr_cnt = 0;
static int my_thr_scnt = 0;
static xmt::mutex lock;

xmt::Thread::ret_t thread_mgr_entry_call( void * )
{
  lock.lock();
  ++my_thr_cnt;
  ++my_thr_scnt;
  lock.unlock();

  lock.lock();
  --my_thr_cnt;
  lock.unlock();

  return 0;
}

int EXAM_IMPL(mt_test::thr_mgr)
{
  xmt::ThreadMgr mgr;

  for ( int i = 0; i < 200; ++i ) {
    mgr.launch( thread_mgr_entry_call, (void *)this, 0, 0, 0 );
  }

  // cerr << "Join!\n";
  mgr.join();

  EXAM_CHECK( my_thr_scnt == 200 );
  EXAM_CHECK( my_thr_cnt == 0 );
  EXAM_CHECK( mgr.size() == 0 );

  return EXAM_RESULT;
}
