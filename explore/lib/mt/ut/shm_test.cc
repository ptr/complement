// -*- C++ -*- Time-stamp: <08/07/02 13:05:03 yeti>

/*
 * Copyright (c) 2006-2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include "shm_test.h"

#include <mt/condition_variable>
#include <mt/shm.h>


#include <sys/shm.h>
#include <sys/wait.h>

#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>

using namespace std;

int EXAM_IMPL(shm_test::shm_segment)
{
  const char fname[] = "/tmp/mt_test.shm";
  try {
    xmt::detail::__shm_alloc<0> seg( 5000, 1024, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    EXAM_CHECK( seg.address() != reinterpret_cast<void *>(-1) );
    seg.deallocate();
    EXAM_CHECK( seg.address() == reinterpret_cast<void *>(-1) );

    // EXAM_REQUIRE( !fs::exists( fname ) );
    struct stat st;
    EXAM_REQUIRE( (stat( fname, &st ) == -1) && (errno == ENOENT) );

    seg.allocate( fname, 1024, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );
    EXAM_CHECK( seg.address() != reinterpret_cast<void *>(-1) );
    seg.deallocate();
    EXAM_CHECK( seg.address() == reinterpret_cast<void *>(-1) );
    // EXAM_CHECK( fs::exists( fname ) ); // well, now I don't remove ref file, because shm segment may be created with another way
    EXAM_CHECK( stat( fname, &st ) == 0 );

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
    // EXAM_CHECK( fs::exists( fname ) );
    // ----

    // fs::remove( fname );
    EXAM_CHECK( unlink( fname ) == 0 );
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
    // fs::remove( fname );
    EXAM_CHECK( unlink( fname ) == 0 );
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

    std::tr2::condition_event_ip& fcnd = *new( shm.allocate( sizeof(std::tr2::condition_event_ip) ) ) std::tr2::condition_event_ip();

    try {
      xmt::fork();

      try {

        // Child code
        if ( fcnd.timed_wait( std::tr2::milliseconds( 800 ) ) ) {
          exit( 0 );
        }

      }
      catch ( ... ) {
      }

      exit( 1 );
    }
    catch ( xmt::fork_in_parent& child ) {
      try {
        EXAM_CHECK( child.pid() > 0 );

        fcnd.notify_one();

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

    (&fcnd)->~__condition_event<true>();
    shm.deallocate( reinterpret_cast<char *>(&fcnd), sizeof(std::tr2::condition_event_ip) );
    seg.deallocate();
    // fs::remove( fname );
    EXAM_CHECK( unlink( fname ) == 0 );
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

    xmt::allocator_shm<std::tr2::condition_event_ip,0> shm;

    std::tr2::condition_event_ip& fcnd = *new ( shm.allocate( 1 ) ) std::tr2::condition_event_ip();
    nm.named( fcnd, test_Condition_Object );

    try {
      xmt::fork();

      int eflag = 0;

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
        std::tr2::condition_event_ip& fcnd_ch = nm_ch.named<std::tr2::condition_event_ip>( test_Condition_Object );
        fcnd_ch.notify_one();
      }
      catch ( const xmt::shm_bad_alloc& err ) {
        EXAM_ERROR_ASYNC_F( err.what(), eflag );
      }
      catch ( const std::invalid_argument& err ) {
        EXAM_ERROR_ASYNC_F( err.what(), eflag );
      }
      catch ( ... ) {
        EXAM_ERROR_ASYNC_F( "Fail in child", eflag );
      }

      exit( eflag );
    }
    catch ( xmt::fork_in_parent& child ) {
      try {
        EXAM_CHECK( child.pid() > 0 );

        EXAM_CHECK( fcnd.timed_wait( std::tr2::milliseconds( 800 ) ) );

        int stat = -1;
        EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
        if ( WIFEXITED(stat) ) {
          EXAM_CHECK( WEXITSTATUS(stat) == 0 );
        } else {
          EXAM_ERROR( "child interrupted" );
        }
      }
      catch ( ... ) {
        EXAM_ERROR( "Fail in parent" );
      }
    }
    catch ( ... ) {
      EXAM_ERROR( "Fail in fork" );
    }
    
    (&fcnd)->~__condition_event<true>();
    shm.deallocate( &fcnd, 1 );
    seg.deallocate();
    // fs::remove( fname );
    EXAM_CHECK( unlink( fname ) == 0 );
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
    try {
      seg1.allocate( fname1, 4*4096, 0, 0660 );
    }
    catch ( xmt::shm_bad_alloc& err2 ) {
      EXAM_ERROR_ASYNC( err2.what() );
      unlink( fname1 );
    }
  }
}

/* ******************************************************
 */
shm_test::~shm_test()
{
  seg1.deallocate();
  // fs::remove( fname1 );
  unlink( fname1 );
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

    xmt::allocator_shm<std::tr2::condition_event_ip,1> shm;

    std::tr2::condition_event_ip& fcnd = *new ( shm.allocate( 1 ) ) std::tr2::condition_event_ip();
    nm.named( fcnd, ObjName );

    try {
      xmt::fork();

      int eflag = 0;

      try {
        xmt::shm_name_mgr<1>& nm_ch = seg1.name_mgr();
        xmt::allocator_shm<std::tr2::condition_event_ip,1> shm_ch;
        std::tr2::condition_event_ip& fcnd_ch = nm_ch.named<std::tr2::condition_event_ip>( ObjName );
        fcnd_ch.notify_one();
        nm_ch.release<std::tr2::condition_event_ip>( ObjName );
      }
      catch ( const std::invalid_argument& err ) {
        EXAM_ERROR_ASYNC_F( err.what(), eflag );
      }
      exit( eflag );
    }
    catch ( xmt::fork_in_parent& child ) {
      EXAM_CHECK( fcnd.timed_wait( std::tr2::milliseconds( 800 ) ) );
      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }
    }
    nm.release<std::tr2::condition_event_ip>( ObjName ); // fcnd should be destroyed here

    std::tr2::condition_event_ip& fcnd1 = *new ( shm.allocate( 1 ) ) std::tr2::condition_event_ip();
    nm.named( fcnd1, ObjName ); // ObjName should be free here

    try {
      xmt::fork();

      int eflag = 0;

      try {
        xmt::shm_name_mgr<1>& nm_ch = seg1.name_mgr();
        xmt::allocator_shm<std::tr2::condition_event_ip,1> shm_ch;
        std::tr2::condition_event_ip& fcnd_ch = nm_ch.named<std::tr2::condition_event_ip>( ObjName );
        fcnd_ch.notify_one();
        nm_ch.release<std::tr2::condition_event_ip>( ObjName );
      }
      catch ( const std::invalid_argument& err ) {
        EXAM_ERROR_ASYNC_F( err.what(), eflag );
      }
      
      exit( eflag );
    }
    catch ( xmt::fork_in_parent& child ) {
      EXAM_CHECK( fcnd1.timed_wait( std::tr2::milliseconds( 800 ) ) );
      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }
    }
    nm.release<std::tr2::condition_event_ip>( ObjName ); // fcnd should be destroyed here

    xmt::allocator_shm<std::tr2::barrier_ip,1> shm_b;
    std::tr2::barrier_ip& b = *new ( shm_b.allocate( 1 ) ) std::tr2::barrier_ip();

    nm.named( b, ObjName ); // ObjName should be free here
    
    try {
      xmt::fork();

      int eflag = 0;
      try {
        xmt::shm_name_mgr<1>& nm_ch = seg1.name_mgr();
        xmt::allocator_shm<std::tr2::barrier_ip,1> shm_ch;
        std::tr2::barrier_ip& b_ch = nm_ch.named<std::tr2::barrier_ip>( ObjName );
        b_ch.wait();
        nm_ch.release<std::tr2::barrier_ip>( ObjName );
      }
      catch ( const std::invalid_argument& err ) {
        EXAM_ERROR_ASYNC_F( err.what(), eflag );
      }
    
      exit( eflag );
    }
    catch ( xmt::fork_in_parent& child ) {
      b.wait();
      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }
    }
    nm.release<std::tr2::barrier_ip>( ObjName ); // barrier should be destroyed here
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }
  catch ( const std::invalid_argument& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

