// -*- C++ -*- Time-stamp: <07/02/19 14:36:01 ptr>

/*
 * Copyright (c) 2006, 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include <boost/test/unit_test.hpp>

#include "mt_test.h"

#include <mt/xmt.h>
#include <mt/shm.h>
#include <mt/thr_mgr.h>

#include <sys/shm.h>
#include <sys/wait.h>

#include <signal.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <iostream>

using namespace std;

using namespace boost::unit_test_framework;
namespace fs = boost::filesystem;

/* ******************************************************
 * Degenerate case: check that one thread pass throw
 * own barrier.
 */
void mt_test::barrier()
{
  xmt::Barrier b( 1 );

  b.wait();
}

/* ******************************************************
 * Start thread, join it.
 */

static int x = 0;

xmt::Thread::ret_code thread_entry_call( void * )
{
  x = 1;

  xmt::Thread::ret_code rt;
  rt.iword = 0;

  return rt;
}

void mt_test::join_test()
{
  BOOST_CHECK( x == 0 );

  xmt::Thread t( thread_entry_call );

  t.join();

  BOOST_CHECK( x == 1 );
}

/* ******************************************************
 * Start two threads, align ones on barrier, join.
 */

xmt::Thread::ret_code thread2_entry_call( void *p )
{
  xmt::Thread::ret_code rt;
  rt.iword = 0;

  xmt::Barrier& b = *reinterpret_cast<xmt::Barrier *>(p);
  b.wait();

  return rt;
}

void mt_test::barrier2()
{
  xmt::Barrier b;

  xmt::Thread t1( thread2_entry_call, &b );
  xmt::Thread t2( thread2_entry_call, &b );

  t2.join();
  t1.join();
}

/* ******************************************************
 * Start two threads, align ones on barrier; one thread
 * relinquish control to other; join (within Thread dtors)
 */

xmt::Thread::ret_code thread3_entry_call( void *p )
{
  xmt::Thread::ret_code rt;
  rt.iword = 0;

  xmt::Barrier& b = *reinterpret_cast<xmt::Barrier *>(p);
  b.wait();
  BOOST_CHECK( xmt::Thread::yield() == 0 );

  return rt;
}

void mt_test::yield()
{
  xmt::Barrier b;

  xmt::Thread t1( thread2_entry_call, &b );
  xmt::Thread t2( thread3_entry_call, &b );
  // .join()'s are in Thread's destructors
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

static xmt::Mutex m1;

xmt::Thread::ret_code thr1( void *p )
{
  xmt::Barrier& b = *reinterpret_cast<xmt::Barrier *>(p);
  b.wait();

  m1.lock();
  BOOST_CHECK( x == 0 );

  xmt::Thread::yield();

  BOOST_CHECK( x == 0 );
  x = 1;

  m1.unlock();

  xmt::Thread::ret_code rt;
  rt.iword = 0;

  return rt;
}

xmt::Thread::ret_code thr2( void *p )
{
  xmt::Barrier& b = *reinterpret_cast<xmt::Barrier *>(p);
  b.wait();
  for ( int i = 0; i < 128; ++i ) {
    xmt::Thread::yield();
  }

  m1.lock();
  BOOST_CHECK( x == 1 );
  x = 2;
  m1.unlock();

  xmt::Thread::ret_code rt;
  rt.iword = 0;

  return rt;
}

void mt_test::mutex_test()
{
  x = 0;
  xmt::Barrier b;

  xmt::Thread t1( thr1, &b );
  xmt::Thread t2( thr2, &b );

  t1.join();
  t2.join();

  BOOST_CHECK( x == 2 );
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
static xmt::Spinlock sl1;

xmt::Thread::ret_code thr1s( void *p )
{
  xmt::Barrier& b = *reinterpret_cast<xmt::Barrier *>(p);
  b.wait();

  sl1.lock();
  BOOST_CHECK( x == 0 );

  xmt::Thread::yield();

  BOOST_CHECK( x == 0 );
  x = 1;

  sl1.unlock();

  xmt::Thread::ret_code rt;
  rt.iword = 0;

  return rt;
}

xmt::Thread::ret_code thr2s( void *p )
{
  xmt::Barrier& b = *reinterpret_cast<xmt::Barrier *>(p);
  b.wait();
  for ( int i = 0; i < 128; ++i ) {
    xmt::Thread::yield();
  }

  sl1.lock();
  BOOST_CHECK( x == 1 );
  x = 2;
  sl1.unlock();

  xmt::Thread::ret_code rt;
  rt.iword = 0;

  return rt;
}

#endif

void mt_test::spinlock_test()
{
#ifdef __FIT_PTHREAD_SPINLOCK
  x = 0;
  xmt::Barrier b;

  xmt::Thread t1( thr1s, &b );
  xmt::Thread t2( thr2s, &b );

  t1.join();
  t2.join();

  BOOST_CHECK( x == 2 );
#endif
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

xmt::__Mutex<true,false> m2;

void recursive()
{
  m2.lock();

  x = 2;
  xmt::Thread::yield();
  BOOST_CHECK( x == 2 );

  m2.unlock();
}

xmt::Thread::ret_code thr1r( void *p )
{
  xmt::Barrier& b = *reinterpret_cast<xmt::Barrier *>(p);
  b.wait();

  m2.lock();

  BOOST_CHECK( x == 0 );
  x = 1;
  xmt::Thread::yield();
  BOOST_CHECK( x == 1 );
  recursive();
  BOOST_CHECK( x == 2 );
  x = 3;

  m2.unlock();

  xmt::Thread::ret_code rt;

  rt.iword = 0;

  return rt;
}

xmt::Thread::ret_code thr2r( void *p )
{
  xmt::Barrier& b = *reinterpret_cast<xmt::Barrier *>(p);
  b.wait();

  for ( int i = 0; i < 128; ++i ) {
    xmt::Thread::yield();
  }

  m2.lock();

  BOOST_CHECK( x == 3 );
  xmt::Thread::yield();
  recursive();  
  BOOST_CHECK( x == 2 );

  m2.unlock();

  xmt::Thread::ret_code rt;

  rt.iword = 0;

  return rt;
}

void mt_test::recursive_mutex_test()
{
  x = 0;
  xmt::Barrier b;

  xmt::Thread t1( thr1r, &b );
  xmt::Thread t2( thr2r, &b );

  t1.join();
  t2.join();

  BOOST_CHECK( x == 2 );
}

/* ****************************************************** */

void mt_test::fork()
{
  shmid_ds ds;
  int id = shmget( 5000, 1024, IPC_CREAT | IPC_EXCL | 0600 );
  BOOST_REQUIRE( id != -1 );
  // if ( id == -1 ) {
  //   cerr << "Error on shmget" << endl;
  // }
  BOOST_REQUIRE( shmctl( id, IPC_STAT, &ds ) != -1 );
  // if ( shmctl( id, IPC_STAT, &ds ) == -1 ) {
  //   cerr << "Error on shmctl" << endl;
  // }
  void *buf = shmat( id, 0, 0 );
  BOOST_REQUIRE( buf != reinterpret_cast<void *>(-1) );
  // if ( buf == reinterpret_cast<void *>(-1) ) {
  //   cerr << "Error on shmat" << endl;
  // }

  xmt::__Condition<true>& fcnd = *new( buf ) xmt::__Condition<true>();
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
      BOOST_CHECK( child.pid() > 0 );

      fcnd.set( true );

      int stat;
      BOOST_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
    }
    catch ( ... ) {
    }
  }
  catch ( ... ) {
  }

  (&fcnd)->~__Condition<true>();

  shmdt( buf );
  shmctl( id, IPC_RMID, &ds );
}

/* ****************************************************** */

void mt_test::pid()
{
  shmid_ds ds;
  int id = shmget( 5000, 1024, IPC_CREAT | IPC_EXCL | 0600 );
  BOOST_REQUIRE( id != -1 );
  // if ( id == -1 ) {
  //   cerr << "Error on shmget" << endl;
  // }
  BOOST_REQUIRE( shmctl( id, IPC_STAT, &ds ) != -1 );
  // if ( shmctl( id, IPC_STAT, &ds ) == -1 ) {
  //   cerr << "Error on shmctl" << endl;
  // }
  void *buf = shmat( id, 0, 0 );
  BOOST_REQUIRE( buf != reinterpret_cast<void *>(-1) );
  // if ( buf == reinterpret_cast<void *>(-1) ) {
  //   cerr << "Error on shmat" << endl;
  // }

  xmt::__Condition<true>& fcnd = *new( buf ) xmt::__Condition<true>();
  fcnd.set( false );
  pid_t my_pid = xmt::getpid();

  try {
    xmt::fork();

    try {

      // Child code 
      BOOST_CHECK( my_pid == xmt::getppid() );
      *reinterpret_cast<pid_t *>(static_cast<char *>(buf) + sizeof(xmt::__Condition<true>)) = xmt::getpid();

      fcnd.set( true );

    }
    catch ( ... ) {
    }

    exit( 0 );
  }
  catch ( xmt::fork_in_parent& child ) {
    try {
      BOOST_CHECK( child.pid() > 0 );

      fcnd.try_wait();

      BOOST_CHECK( *reinterpret_cast<pid_t *>(static_cast<char *>(buf) + sizeof(xmt::__Condition<true>)) == child.pid() );

      int stat;
      BOOST_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
    }
    catch ( ... ) {
    }
  }
  catch ( ... ) {
  }

  (&fcnd)->~__Condition<true>();

  shmdt( buf );
  shmctl( id, IPC_RMID, &ds );
}

/* ****************************************************** */

void mt_test::shm_segment()
{
  const char fname[] = "/tmp/mt_test.shm";
  try {
    xmt::detail::__shm_alloc<0> seg( 5000, 1024, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    BOOST_CHECK( seg.address() != reinterpret_cast<void *>(-1) );
    seg.deallocate();
    BOOST_CHECK( seg.address() == reinterpret_cast<void *>(-1) );

    BOOST_REQUIRE( !fs::exists( fname ) );

    seg.allocate( fname, 1024, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );
    BOOST_CHECK( seg.address() != reinterpret_cast<void *>(-1) );
    seg.deallocate();
    BOOST_CHECK( seg.address() == reinterpret_cast<void *>(-1) );
    BOOST_CHECK( fs::exists( fname ) ); // well, now I don't remove ref file, because shm segment may be created with another way

    // not exclusive, should pass
    seg.allocate( fname, 1024, xmt::shm_base::create, 0660 );
    BOOST_CHECK( seg.address() != reinterpret_cast<void *>(-1) );
    try {
      // This instance has segment in usage, should throw
      seg.allocate( fname, 1024, 0, 0660 );
      BOOST_CHECK( false );
    }
    catch ( xmt::shm_bad_alloc& err ) {
      BOOST_CHECK( true ); // Ok
    }

    /*

    I will treat another instanse (<1>) as interface to another
    segment, so this sample not work.

    try {
      // But this is another instanse, it's ok:
      xmt::detail::__shm_alloc<1> seg1( fname, 1024, 0, 0660 );
      BOOST_CHECK( seg1.address() != reinterpret_cast<void *>(-1) );
      // Don't call seg1.deallocate() here, it destroy 
    }
    catch ( xmt::shm_bad_alloc& err ) {
      BOOST_CHECK( false ); // Fail
    }
    */

    seg.deallocate();
    BOOST_CHECK( seg.address() == reinterpret_cast<void *>(-1) );

    // ----
    try {
      // exclusive, should throw
      seg.allocate( fname, 1024, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );
      BOOST_CHECK( false ); // Fail, should throw
    }
    catch ( xmt::shm_bad_alloc& err ) {
      BOOST_CHECK( true ); // Ok
    }
    BOOST_CHECK( fs::exists( fname ) );
    // ----

    fs::remove( fname );
  }
  catch ( xmt::shm_bad_alloc& err ) {
    BOOST_CHECK_MESSAGE( false, "error report: " << err.what() );
  }
}

/* ****************************************************** */

void mt_test::shm_alloc()
{
  const char fname[] = "/tmp/mt_test.shm";
  try {
    xmt::shm_alloc<0> seg;

    seg.allocate( fname, 7000, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    {
      xmt::allocator_shm<char,0> shmall;
      size_t sz = shmall.max_size();
      // two blocks
      char *ch1 = shmall.allocate( 3500 );
      BOOST_CHECK( ch1 != 0 );
      char *ch2 = shmall.allocate( 3500 );
      BOOST_CHECK( ch2 != 0 );
      try {
        // try to allocate third block, not enough room
        char *ch3 = shmall.allocate( 8 * 1024 - 7000 );
        BOOST_CHECK( false );
      }
      catch ( xmt::shm_bad_alloc& err ) {
        BOOST_CHECK( true );
      }
      // free first blocks
      shmall.deallocate( ch1, 3500 );
      ch1 = shmall.allocate( 3500 );
      // allocate [first] block again
      BOOST_CHECK( ch1 != 0 );
      // free second block
      shmall.deallocate( ch2, 3500 );
      // allocate [second] block again
      ch2 = shmall.allocate( 3500 );
      BOOST_CHECK( ch2 != 0 );
      // free both blocks
      shmall.deallocate( ch1, 3500 );
      shmall.deallocate( ch2, 3500 );
      // allocate big block, enough for initial memory chunk
      ch1 = shmall.allocate( 7000 );
      BOOST_CHECK( ch1 != 0 );
      // free it
      shmall.deallocate( ch1, 7000 );
      // allocate block of maximum size
      ch1 = shmall.allocate( sz );
      BOOST_CHECK( ch1 != 0 );
      // free it
      shmall.deallocate( ch1, sz );
      // allocate block, enough for initial memory chunk
      ch1 = shmall.allocate( 7000 );
      BOOST_CHECK( ch1 != 0 );
      // free it
      shmall.deallocate( ch1, 7000 );
      ch1 = shmall.allocate( 3000 );
      BOOST_CHECK( ch1 != 0 );
      ch2 = shmall.allocate( 400 );
      BOOST_CHECK( ch2 != 0 );
      char *ch3 = shmall.allocate( 3500 );
      BOOST_CHECK( ch3 != 0 );
      shmall.deallocate( ch1, 3000 );
      shmall.deallocate( ch2, 400 );
      shmall.deallocate( ch3, 3500 );
      ch1 = shmall.allocate( sz );
      BOOST_CHECK( ch1 != 0 );
      shmall.deallocate( ch1, sz );
    }
    seg.deallocate();
    fs::remove( fname );
  }
  catch ( xmt::shm_bad_alloc& err ) {
    BOOST_CHECK_MESSAGE( false, "error report: " << err.what() );
  }
}


/* ******************************************************
 * This test is similar  mt_test::fork() above, but instead plain shm_*
 * functions it use allocator based on shared memory segment
 */
void mt_test::fork_shm()
{
  const char fname[] = "/tmp/mt_test.shm";
  try {
    xmt::shm_alloc<0> seg;

    seg.allocate( fname, 1024, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );
    xmt::allocator_shm<char,0> shm;

    xmt::__Condition<true>& fcnd = *new( shm.allocate( sizeof(xmt::__Condition<true>) ) ) xmt::__Condition<true>();
    fcnd.set( false );
    try {
      xmt::fork();

      try {

        // Child code
        fcnd.try_wait();

      }
      catch ( ... ) {
      }

      exit( 0 );
    }
    catch ( xmt::fork_in_parent& child ) {
      try {
        BOOST_CHECK( child.pid() > 0 );

        fcnd.set( true );

        int stat;
        BOOST_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      }
      catch ( ... ) {
      }
    }
    catch ( ... ) {
    }

    (&fcnd)->~__Condition<true>();
    shm.deallocate( reinterpret_cast<char *>(&fcnd), sizeof(xmt::__Condition<true>) );
    seg.deallocate();
    fs::remove( fname );
  }
  catch (  xmt::shm_bad_alloc& err ) {
    BOOST_CHECK_MESSAGE( false, "error report: " << err.what() );
  }
}

/* ******************************************************
 * Test: how to take named object in shared memory segment
 */
void mt_test::shm_named_obj()
{
  const char fname[] = "/tmp/mt_test.shm";
  enum {
    test_Condition_Object = 1
  };
  try {
    xmt::shm_alloc<0> seg;

    seg.allocate( fname, 4*4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );
    xmt::shm_name_mgr<0>& nm = seg.name_mgr();

    xmt::allocator_shm<xmt::__Condition<true>,0> shm;

    xmt::__Condition<true>& fcnd = *new ( shm.allocate( 1 ) ) xmt::__Condition<true>();
    nm.named( fcnd, test_Condition_Object );
    fcnd.set( false );

    try {
      xmt::fork();

      try {

        // Child code 
        xmt::shm_alloc<0> seg_ch;

        if ( seg_ch.max_size() == 0 ) { // just illustration, if seg and seg_ch
                                        // (really xmt::shm_alloc<0>)
                                        // in totally different address spaces
          // in our case xmt::shm_name_mgr<0> instance derived from parent
          // process
          seg.allocate( fname, 4*4096, 0, 0660 );
        }
        
        xmt::shm_name_mgr<0>& nm_ch = seg_ch.name_mgr();
        xmt::__Condition<true>& fcnd_ch = nm_ch.named<xmt::__Condition<true> >( test_Condition_Object );
        fcnd_ch.set( true );
      }
      catch ( const xmt::shm_bad_alloc& err ) {
        BOOST_CHECK_MESSAGE( false, "Fail in child: " << err.what() );
      }
      catch ( const std::invalid_argument& err ) {
        BOOST_CHECK_MESSAGE( false, "Fail in child: " << err.what() );
      }
      catch ( ... ) {
        BOOST_CHECK_MESSAGE( false, "Fail in child" );
      }

      exit( 0 );
    }
    catch ( xmt::fork_in_parent& child ) {
      try {
        BOOST_CHECK( child.pid() > 0 );

        fcnd.try_wait();

        int stat;
        BOOST_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      }
      catch ( ... ) {
        BOOST_CHECK_MESSAGE( false, "Fail in parent" );
      }
    }
    catch ( ... ) {
      BOOST_CHECK_MESSAGE( false, "Fail in fork" );
    }
    
    (&fcnd)->~__Condition<true>();
    shm.deallocate( &fcnd, 1 );
    seg.deallocate();
    fs::remove( fname );
  }
  catch ( xmt::shm_bad_alloc& err ) {
    BOOST_CHECK_MESSAGE( false, "error report: " << err.what() );
  }
}

/* ******************************************************
 */
static const char fname1[] = "/tmp/mt_test.shm.1";
xmt::shm_alloc<1> seg1;

void mt_test::shm_init()
{
  seg1.allocate( fname1, 4*4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );
}

/* ******************************************************
 */
void mt_test::shm_finit()
{
  seg1.deallocate();
  fs::remove( fname1 );
}

/* ******************************************************
 */

void mt_test::shm_named_obj_more()
{
  enum {
    ObjName = 1
  };

  try {
    xmt::shm_name_mgr<1>& nm = seg1.name_mgr();

    xmt::allocator_shm<xmt::__Condition<true>,1> shm;

    xmt::__Condition<true>& fcnd = *new ( shm.allocate( 1 ) ) xmt::__Condition<true>();
    nm.named( fcnd, ObjName );
    fcnd.set( false );

    try {
      xmt::fork();

      xmt::shm_name_mgr<1>& nm_ch = seg1.name_mgr();
      xmt::allocator_shm<xmt::__Condition<true>,1> shm_ch;
      xmt::__Condition<true>& fcnd_ch = nm_ch.named<xmt::__Condition<true> >( ObjName );
      fcnd_ch.set( true );
      nm_ch.release<xmt::__Condition<true> >( ObjName );
    
      exit( 0 );
    }
    catch ( xmt::fork_in_parent& child ) {
      fcnd.try_wait();
      int stat;
      BOOST_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
    }
    nm.release<xmt::__Condition<true> >( ObjName ); // fcnd should be destroyed here

    xmt::__Condition<true>& fcnd1 = *new ( shm.allocate( 1 ) ) xmt::__Condition<true>();
    nm.named( fcnd1, ObjName ); // ObjName should be free here
    fcnd1.set( false );

    try {
      xmt::fork();

      xmt::shm_name_mgr<1>& nm_ch = seg1.name_mgr();
      xmt::allocator_shm<xmt::__Condition<true>,1> shm_ch;
      xmt::__Condition<true>& fcnd_ch = nm_ch.named<xmt::__Condition<true> >( ObjName );
      fcnd_ch.set( true );
      nm_ch.release<xmt::__Condition<true> >( ObjName );
    
      exit( 0 );
    }
    catch ( xmt::fork_in_parent& child ) {
      fcnd1.try_wait();
      int stat;
      BOOST_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
    }
    nm.release<xmt::__Condition<true> >( ObjName ); // fcnd should be destroyed here

    xmt::allocator_shm<xmt::__Barrier<true>,1> shm_b;
    xmt::__Barrier<true>& b = *new ( shm_b.allocate( 1 ) ) xmt::__Barrier<true>();

    nm.named( b, ObjName ); // ObjName should be free here
    
    try {
      xmt::fork();

      xmt::shm_name_mgr<1>& nm_ch = seg1.name_mgr();
      xmt::allocator_shm<xmt::__Barrier<true>,1> shm_ch;
      xmt::__Barrier<true>& b_ch = nm_ch.named<xmt::__Barrier<true> >( ObjName );
      b_ch.wait();
      nm_ch.release<xmt::__Barrier<true> >( ObjName );
    
      exit( 0 );      
    }
    catch ( xmt::fork_in_parent& child ) {
      b.wait();
      int stat;
      BOOST_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
    }
    nm.release<xmt::__Barrier<true> >( ObjName ); // barrier should be destroyed here
  }
  catch ( xmt::shm_bad_alloc& err ) {
    BOOST_CHECK_MESSAGE( false, "error report: " << err.what() );
  }
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
static xmt::Mutex lock;

xmt::Thread::ret_code thread_mgr_entry_call( void * )
{
  lock.lock();
  ++my_thr_cnt;
  ++my_thr_scnt;
  lock.unlock();

  xmt::Thread::ret_code rt;
  rt.iword = 0;

  lock.lock();
  --my_thr_cnt;
  lock.unlock();

  return rt;
}

void mt_test::thr_mgr()
{
  xmt::ThreadMgr mgr;

  for ( int i = 0; i < 200; ++i ) {
    mgr.launch( thread_mgr_entry_call, (void *)this, 0, 0, 0 );
  }

  // cerr << "Join!\n";
  mgr.join();

  BOOST_CHECK( my_thr_scnt == 200 );
  BOOST_CHECK( my_thr_cnt == 0 );
  BOOST_CHECK( mgr.size() == 0 );
}
