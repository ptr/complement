// -*- C++ -*- Time-stamp: <08/12/22 11:39:53 ptr>

/*
 * Copyright (c) 2004, 2006-2008
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include "flock_test.h"
#include <string>
#include <fstream>
#include <functional>
#include <mt/uid.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <mt/lfstream>
#include <mt/shm.h>

using namespace std;

flock_test::~flock_test()
{
  if ( !fname.empty() ) {
    unlink( fname.c_str() );
  }
}

int EXAM_IMPL(flock_test::create)
{
  fname = "/tmp/xmt-";
  fname += xmt::uid_str();

  int fd = open( fname.c_str(), O_RDWR | O_CREAT | O_APPEND, 00666 );
  EXAM_CHECK( fd >= 0 );
  
  char buf[1024];
  const int factor = 4;

  fill( buf, buf + sizeof(buf), 'a' );

  int cnt = 0;

  for ( int i = 0; i < factor; ++i ) {
    cnt += write( fd, buf, sizeof(buf) );
  }

  EXAM_CHECK( cnt == (factor * sizeof(buf)) );

  EXAM_CHECK( close( fd ) == 0 );

  return EXAM_RESULT;
}

int EXAM_IMPL(flock_test::read_lock)
{
  try {
    xmt::shm_alloc<0> seg;

    seg.allocate( 70000, 1024, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );
    xmt::allocator_shm<char,0> shm;

    std::tr2::condition_event_ip& fcnd = *new( shm.allocate( sizeof(std::tr2::condition_event_ip) ) ) std::tr2::condition_event_ip();
    std::tr2::condition_event_ip& fcnd2 = *new( shm.allocate( sizeof(std::tr2::condition_event_ip) ) ) std::tr2::condition_event_ip();

    try {
      xmt::fork();

      try {

        lfstream f( fname.c_str() );

        EXAM_CHECK_ASYNC( f.is_open() );
        EXAM_CHECK_ASYNC( f.good() );

        f.rdlock();

        fcnd.notify_one();

        EXAM_CHECK_ASYNC( fcnd2.timed_wait( std::tr2::milliseconds( 800 ) ) );

        f.unlock();

        //  EXAM_CHECK_ASYNC( fcnd2.timed_wait( std::tr2::milliseconds( 800 ) ) );
      }
      catch ( ... ) {
      }

      exit( 0 );
    }
    catch ( xmt::fork_in_parent& child ) {
      try {
        EXAM_CHECK( child.pid() > 0 );

        EXAM_CHECK( fcnd.timed_wait( std::tr2::milliseconds( 800 ) ) );

        lfstream f( fname.c_str() );

        EXAM_CHECK_ASYNC( f.is_open() );
        EXAM_CHECK_ASYNC( f.good() );

        f.rdlock();

        fcnd2.notify_one();

        f.unlock();

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
    (&fcnd2)->~__condition_event<true>();
    shm.deallocate( reinterpret_cast<char *>(&fcnd2), sizeof(std::tr2::condition_event_ip) );
    (&fcnd)->~__condition_event<true>();
    shm.deallocate( reinterpret_cast<char *>(&fcnd), sizeof(std::tr2::condition_event_ip) );
    seg.deallocate();
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(flock_test::write_lock)
{
  try {
    xmt::shm_alloc<0> seg;

    seg.allocate( 70000, 1024, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );
    xmt::allocator_shm<char,0> shm;

    std::tr2::condition_event_ip& fcnd = *new( shm.allocate( sizeof(std::tr2::condition_event_ip) ) ) std::tr2::condition_event_ip();
    std::tr2::condition_event_ip& fcnd2 = *new( shm.allocate( sizeof(std::tr2::condition_event_ip) ) ) std::tr2::condition_event_ip();

    try {
      xmt::fork();

      try {

        lfstream f( fname.c_str() );

        EXAM_CHECK_ASYNC( f.is_open() );
        EXAM_CHECK_ASYNC( f.good() );

        f.lock();

        fcnd.notify_one();

        EXAM_CHECK_ASYNC( !fcnd2.timed_wait( std::tr2::milliseconds( 800 ) ) );

        f.unlock();

        EXAM_CHECK_ASYNC( fcnd2.timed_wait( std::tr2::milliseconds( 800 ) ) );
      }
      catch ( ... ) {
      }

      exit( 0 );
    }
    catch ( xmt::fork_in_parent& child ) {
      try {
        EXAM_CHECK( child.pid() > 0 );

        EXAM_CHECK( fcnd.timed_wait( std::tr2::milliseconds( 800 ) ) );

        lfstream f( fname.c_str() );

        EXAM_CHECK_ASYNC( f.is_open() );
        EXAM_CHECK_ASYNC( f.good() );

        f.lock();

        fcnd2.notify_one();

        f.unlock();

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
    (&fcnd2)->~__condition_event<true>();
    shm.deallocate( reinterpret_cast<char *>(&fcnd2), sizeof(std::tr2::condition_event_ip) );
    (&fcnd)->~__condition_event<true>();
    shm.deallocate( reinterpret_cast<char *>(&fcnd), sizeof(std::tr2::condition_event_ip) );
    seg.deallocate();
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}
