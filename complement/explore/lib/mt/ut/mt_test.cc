// -*- C++ -*- Time-stamp: <07/07/17 09:58:22 ptr>

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

#include <sys/shm.h>
#include <sys/wait.h>

#include <signal.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <iostream>

using namespace std;
namespace fs = boost::filesystem;

/* ******************************************************
 * Degenerate case: check that one thread pass throw
 * own barrier.
 */
int EXAM_IMPL(mt_test::barrier)
{
  xmt::barrier b( 1 );

  b.wait();
}

/* ******************************************************
 * Start thread, join it. Check return value.
 */

static int x = 0;

xmt::Thread::ret_code thread_entry_call( void * )
{
  x = 1;

  xmt::Thread::ret_code rt;
  rt.iword = 2;

  return rt;
}

int EXAM_IMPL(mt_test::join_test)
{
  EXAM_CHECK( x == 0 );

  xmt::Thread t( thread_entry_call );

  EXAM_CHECK( t.join().iword == 2 );

  EXAM_CHECK( x == 1 );

  return EXAM_RESULT;
}

/* ******************************************************
 * Start two threads, align ones on barrier, join.
 */

xmt::Thread::ret_code thread2_entry_call( void *p )
{
  xmt::Thread::ret_code rt;
  rt.iword = 1;

  xmt::barrier& b = *reinterpret_cast<xmt::barrier *>(p);
  b.wait();

  return rt;
}

int EXAM_IMPL(mt_test::barrier2)
{
  xmt::barrier b;

  xmt::Thread t1( thread2_entry_call, &b );
  xmt::Thread t2( thread2_entry_call, &b );

  EXAM_CHECK( t2.join().iword == 1 );
  EXAM_CHECK( t1.join().iword == 1 );

  return EXAM_RESULT;
}

/* ******************************************************
 * Start two threads, align ones on barrier; one thread
 * relinquish control to other; join (within Thread dtors)
 */

xmt::Thread::ret_code thread3_entry_call( void *p )
{
  xmt::Thread::ret_code rt;
  rt.iword = 0;

  xmt::barrier& b = *reinterpret_cast<xmt::barrier *>(p);
  b.wait();
  EXAM_CHECK_ASYNC( xmt::Thread::yield() == 0 );

  return rt;
}

int EXAM_IMPL(mt_test::yield)
{
  {
    xmt::barrier b;

    xmt::Thread t1( thread2_entry_call, &b );
    xmt::Thread t2( thread3_entry_call, &b );
    // .join()'s are in Thread's destructors
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

xmt::Thread::ret_code thr1( void *p )
{
  xmt::barrier& b = *reinterpret_cast<xmt::barrier *>(p);
  b.wait();

  m1.lock();
  EXAM_CHECK_ASYNC( x == 0 );

  xmt::Thread::yield();

  EXAM_CHECK_ASYNC( x == 0 );
  x = 1;

  m1.unlock();

  xmt::Thread::ret_code rt;
  rt.iword = 0;

  return rt;
}

xmt::Thread::ret_code thr2( void *p )
{
  xmt::barrier& b = *reinterpret_cast<xmt::barrier *>(p);
  b.wait();
  for ( int i = 0; i < 128; ++i ) {
    xmt::Thread::yield();
  }

  m1.lock();
  EXAM_CHECK_ASYNC( x == 1 );
  x = 2;
  m1.unlock();

  xmt::Thread::ret_code rt;
  rt.iword = 0;

  return rt;
}

int EXAM_IMPL(mt_test::mutex_test)
{
  x = 0;
  xmt::barrier b;

  xmt::Thread t1( thr1, &b );
  xmt::Thread t2( thr2, &b );

  t1.join();
  t2.join();

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

xmt::Thread::ret_code thr1s( void *p )
{
  xmt::barrier& b = *reinterpret_cast<xmt::barrier *>(p);
  b.wait();

  sl1.lock();
  EXAM_CHECK_ASYNC( x == 0 );

  xmt::Thread::yield();

  EXAM_CHECK_ASYNC( x == 0 );
  x = 1;

  sl1.unlock();

  xmt::Thread::ret_code rt;
  rt.iword = 0;

  return rt;
}

xmt::Thread::ret_code thr2s( void *p )
{
  xmt::barrier& b = *reinterpret_cast<xmt::barrier *>(p);
  b.wait();
  for ( int i = 0; i < 128; ++i ) {
    xmt::Thread::yield();
  }

  sl1.lock();
  EXAM_CHECK_ASYNC( x == 1 );
  x = 2;
  sl1.unlock();

  xmt::Thread::ret_code rt;
  rt.iword = 0;

  return rt;
}

#endif

int EXAM_IMPL(mt_test::spinlock_test)
{
#ifdef __FIT_PTHREAD_SPINLOCK
  x = 0;
  xmt::barrier b;

  xmt::Thread t1( thr1s, &b );
  xmt::Thread t2( thr2s, &b );

  t1.join();
  t2.join();

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

xmt::Thread::ret_code thr1r( void *p )
{
  xmt::barrier& b = *reinterpret_cast<xmt::barrier *>(p);
  b.wait();

  m2.lock();

  EXAM_CHECK_ASYNC( x == 0 );
  x = 1;
  xmt::Thread::yield();
  EXAM_CHECK_ASYNC( x == 1 );
  recursive();
  EXAM_CHECK_ASYNC( x == 2 );
  x = 3;

  m2.unlock();

  xmt::Thread::ret_code rt;

  rt.iword = 0;

  return rt;
}

xmt::Thread::ret_code thr2r( void *p )
{
  xmt::barrier& b = *reinterpret_cast<xmt::barrier *>(p);
  b.wait();

  for ( int i = 0; i < 128; ++i ) {
    xmt::Thread::yield();
  }

  m2.lock();

  EXAM_CHECK_ASYNC( x == 3 );
  xmt::Thread::yield();
  recursive();  
  EXAM_CHECK_ASYNC( x == 2 );

  m2.unlock();

  xmt::Thread::ret_code rt;

  rt.iword = 0;

  return rt;
}

int EXAM_IMPL(mt_test::recursive_mutex_test)
{
  x = 0;
  xmt::barrier b;

  xmt::Thread t1( thr1r, &b );
  xmt::Thread t2( thr2r, &b );

  t1.join();
  t2.join();

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

      int stat;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
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

    try {

      // Child code 
      EXAM_CHECK_ASYNC( my_pid == xmt::getppid() );
      *reinterpret_cast<pid_t *>(static_cast<char *>(buf) + sizeof(xmt::__condition<true>)) = xmt::getpid();

      fcnd.set( true );

    }
    catch ( ... ) {
    }

    exit( 0 );
  }
  catch ( xmt::fork_in_parent& child ) {
    try {
      EXAM_CHECK( child.pid() > 0 );

      fcnd.try_wait();

      EXAM_CHECK( *reinterpret_cast<pid_t *>(static_cast<char *>(buf) + sizeof(xmt::__condition<true>)) == child.pid() );

      int stat;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
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

int EXAM_IMPL(shm_test::shm_segment)
{
  const char fname[] = "/tmp/mt_test.shm";
  try {
    xmt::detail::__shm_alloc<0> seg( 5000, 1024, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    EXAM_CHECK( seg.address() != reinterpret_cast<void *>(-1) );
    seg.deallocate();
    EXAM_CHECK( seg.address() == reinterpret_cast<void *>(-1) );

    EXAM_REQUIRE( !fs::exists( fname ) );

    seg.allocate( fname, 1024, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );
    EXAM_CHECK( seg.address() != reinterpret_cast<void *>(-1) );
    seg.deallocate();
    EXAM_CHECK( seg.address() == reinterpret_cast<void *>(-1) );
    EXAM_CHECK( fs::exists( fname ) ); // well, now I don't remove ref file, because shm segment may be created with another way

    // not exclusive, should pass
    seg.allocate( fname, 1024, xmt::shm_base::create, 0660 );
    EXAM_CHECK( seg.address() != reinterpret_cast<void *>(-1) );
    try {
      // This instance has segment in usage, should throw
      seg.allocate( fname, 1024, 0, 0660 );
      EXAM_CHECK( false );
    }
    catch ( xmt::shm_bad_alloc& err ) {
      EXAM_CHECK( true ); // Ok
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
    EXAM_CHECK( seg.address() == reinterpret_cast<void *>(-1) );

    // ----
    try {
      // exclusive, should throw
      seg.allocate( fname, 1024, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );
      EXAM_CHECK( false ); // Fail, should throw
    }
    catch ( xmt::shm_bad_alloc& err ) {
      EXAM_CHECK( true ); // Ok
    }
    EXAM_CHECK( fs::exists( fname ) );
    // ----

    fs::remove( fname );
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

/* ****************************************************** */

int EXAM_IMPL(shm_test::shm_alloc)
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
      EXAM_CHECK( ch1 != 0 );
      char *ch2 = shmall.allocate( 3500 );
      EXAM_CHECK( ch2 != 0 );
      try {
        // try to allocate third block, not enough room
        char *ch3 = shmall.allocate( 8 * 1024 - 7000 );
        EXAM_CHECK( false );
      }
      catch ( xmt::shm_bad_alloc& err ) {
        EXAM_CHECK( true );
      }
      // free first blocks
      shmall.deallocate( ch1, 3500 );
      ch1 = shmall.allocate( 3500 );
      // allocate [first] block again
      EXAM_CHECK( ch1 != 0 );
      // free second block
      shmall.deallocate( ch2, 3500 );
      // allocate [second] block again
      ch2 = shmall.allocate( 3500 );
      EXAM_CHECK( ch2 != 0 );
      // free both blocks
      shmall.deallocate( ch1, 3500 );
      shmall.deallocate( ch2, 3500 );
      // allocate big block, enough for initial memory chunk
      ch1 = shmall.allocate( 7000 );
      EXAM_CHECK( ch1 != 0 );
      // free it
      shmall.deallocate( ch1, 7000 );
      // allocate block of maximum size
      ch1 = shmall.allocate( sz );
      EXAM_CHECK( ch1 != 0 );
      // free it
      shmall.deallocate( ch1, sz );
      // allocate block, enough for initial memory chunk
      ch1 = shmall.allocate( 7000 );
      EXAM_CHECK( ch1 != 0 );
      // free it
      shmall.deallocate( ch1, 7000 );
      ch1 = shmall.allocate( 3000 );
      EXAM_CHECK( ch1 != 0 );
      ch2 = shmall.allocate( 400 );
      EXAM_CHECK( ch2 != 0 );
      char *ch3 = shmall.allocate( 3500 );
      EXAM_CHECK( ch3 != 0 );
      shmall.deallocate( ch1, 3000 );
      shmall.deallocate( ch2, 400 );
      shmall.deallocate( ch3, 3500 );
      ch1 = shmall.allocate( sz );
      EXAM_CHECK( ch1 != 0 );
      shmall.deallocate( ch1, sz );
    }
    seg.deallocate();
    fs::remove( fname );
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}


/* ******************************************************
 * This test is similar  mt_test::fork() above, but instead plain shm_*
 * functions it use allocator based on shared memory segment
 */
int EXAM_IMPL(shm_test::fork_shm)
{
  const char fname[] = "/tmp/mt_test.shm";
  try {
    xmt::shm_alloc<0> seg;

    seg.allocate( fname, 1024, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );
    xmt::allocator_shm<char,0> shm;

    xmt::__condition<true>& fcnd = *new( shm.allocate( sizeof(xmt::__condition<true>) ) ) xmt::__condition<true>();
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
        EXAM_CHECK( child.pid() > 0 );

        fcnd.set( true );

        int stat;
        EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      }
      catch ( ... ) {
      }
    }
    catch ( ... ) {
    }

    (&fcnd)->~__condition<true>();
    shm.deallocate( reinterpret_cast<char *>(&fcnd), sizeof(xmt::__condition<true>) );
    seg.deallocate();
    fs::remove( fname );
  }
  catch (  xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

/* ******************************************************
 * Test: how to take named object in shared memory segment
 */
int EXAM_IMPL(shm_test::shm_named_obj)
{
  const char fname[] = "/tmp/mt_test.shm";
  enum {
    test_Condition_Object = 1
  };
  try {
    xmt::shm_alloc<0> seg;

    seg.allocate( fname, 4*4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );
    xmt::shm_name_mgr<0>& nm = seg.name_mgr();

    xmt::allocator_shm<xmt::__condition<true>,0> shm;

    xmt::__condition<true>& fcnd = *new ( shm.allocate( 1 ) ) xmt::__condition<true>();
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
        xmt::__condition<true>& fcnd_ch = nm_ch.named<xmt::__condition<true> >( test_Condition_Object );
        fcnd_ch.set( true );
      }
      catch ( const xmt::shm_bad_alloc& err ) {
        EXAM_ERROR_ASYNC( err.what() );
      }
      catch ( const std::invalid_argument& err ) {
        EXAM_ERROR_ASYNC( err.what() );
      }
      catch ( ... ) {
        EXAM_ERROR_ASYNC( "Fail in child" );
      }

      exit( 0 );
    }
    catch ( xmt::fork_in_parent& child ) {
      try {
        EXAM_CHECK( child.pid() > 0 );

        fcnd.try_wait();

        int stat;
        EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      }
      catch ( ... ) {
        EXAM_ERROR( "Fail in parent" );
      }
    }
    catch ( ... ) {
      EXAM_ERROR( "Fail in fork" );
    }
    
    (&fcnd)->~__condition<true>();
    shm.deallocate( &fcnd, 1 );
    seg.deallocate();
    fs::remove( fname );
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

/* ******************************************************
 */
const char shm_test::fname1[] = "/tmp/mt_test.shm.1";

shm_test::shm_test()
{
  try {
    seg1.allocate( fname1, 4*4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR_ASYNC( err.what() );
  }
}

/* ******************************************************
 */
shm_test::~shm_test()
{
  seg1.deallocate();
  fs::remove( fname1 );
}

/* ******************************************************
 */

int EXAM_IMPL(shm_test::shm_named_obj_more)
{
  enum {
    ObjName = 1
  };

  try {
    xmt::shm_name_mgr<1>& nm = seg1.name_mgr();

    xmt::allocator_shm<xmt::__condition<true>,1> shm;

    xmt::__condition<true>& fcnd = *new ( shm.allocate( 1 ) ) xmt::__condition<true>();
    nm.named( fcnd, ObjName );
    fcnd.set( false );

    try {
      xmt::fork();

      try {
        xmt::shm_name_mgr<1>& nm_ch = seg1.name_mgr();
        xmt::allocator_shm<xmt::__condition<true>,1> shm_ch;
        xmt::__condition<true>& fcnd_ch = nm_ch.named<xmt::__condition<true> >( ObjName );
        fcnd_ch.set( true );
        nm_ch.release<xmt::__condition<true> >( ObjName );
      }
      catch ( const std::invalid_argument& err ) {
        EXAM_ERROR_ASYNC( err.what() );
      }
      exit( 0 );
    }
    catch ( xmt::fork_in_parent& child ) {
      fcnd.try_wait();
      int stat;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
    }
    nm.release<xmt::__condition<true> >( ObjName ); // fcnd should be destroyed here

    xmt::__condition<true>& fcnd1 = *new ( shm.allocate( 1 ) ) xmt::__condition<true>();
    nm.named( fcnd1, ObjName ); // ObjName should be free here
    fcnd1.set( false );

    try {
      xmt::fork();

      try {
        xmt::shm_name_mgr<1>& nm_ch = seg1.name_mgr();
        xmt::allocator_shm<xmt::__condition<true>,1> shm_ch;
        xmt::__condition<true>& fcnd_ch = nm_ch.named<xmt::__condition<true> >( ObjName );
        fcnd_ch.set( true );
        nm_ch.release<xmt::__condition<true> >( ObjName );
      }
      catch ( const std::invalid_argument& err ) {
        EXAM_ERROR_ASYNC( err.what() );
      }
      
      exit( 0 );
    }
    catch ( xmt::fork_in_parent& child ) {
      fcnd1.try_wait();
      int stat;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
    }
    nm.release<xmt::__condition<true> >( ObjName ); // fcnd should be destroyed here

    xmt::allocator_shm<xmt::__barrier<true>,1> shm_b;
    xmt::__barrier<true>& b = *new ( shm_b.allocate( 1 ) ) xmt::__barrier<true>();

    nm.named( b, ObjName ); // ObjName should be free here
    
    try {
      xmt::fork();

      try {
        xmt::shm_name_mgr<1>& nm_ch = seg1.name_mgr();
        xmt::allocator_shm<xmt::__barrier<true>,1> shm_ch;
        xmt::__barrier<true>& b_ch = nm_ch.named<xmt::__barrier<true> >( ObjName );
        b_ch.wait();
        nm_ch.release<xmt::__barrier<true> >( ObjName );
      }
      catch ( const std::invalid_argument& err ) {
        EXAM_ERROR_ASYNC( err.what() );
      }
    
      exit( 0 );      
    }
    catch ( xmt::fork_in_parent& child ) {
      b.wait();
      int stat;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
    }
    nm.release<xmt::__barrier<true> >( ObjName ); // barrier should be destroyed here
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }

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
