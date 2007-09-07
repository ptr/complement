// -*- C++ -*- Time-stamp: <07/09/06 11:17:21 ptr>

/*
 *
 * Copyright (c) 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "sockios_perf.h"
#include <exam/suite.h>

#include <sockios/sockstream>
#include <sockios/sockmgr.h>

#include <mt/shm.h>
#include <sys/wait.h>
#include <signal.h>
#include <algorithm>

using namespace std;

sockios_perf::sockios_perf() // :
// fname( "/tmp/sockios_perf.shm" )
{
//  try {
//    seg.allocate( fname.c_str(), 4*4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0600 );
//  }
//  catch ( const xmt::shm_bad_alloc& err ) {
//    EXAM_ERROR_ASYNC( err.what() );
//  }
}

sockios_perf::~sockios_perf()
{
//  seg.deallocate();
//  unlink( fname.c_str() );
}

/* ************************************************************ */

class SrvR
{
  public:
    SrvR( sockstream& )
      { }

    ~SrvR()
      { }

    void connect( sockstream& s )
      {
        s.read( buf, 1024*8 );
        EXAM_CHECK_ASYNC( s.good() );
      }

    void close()
      { }

  private:
    char buf[10240];
};

class SrvW
{
  public:
    SrvW( sockstream& s )
      {
        fill( buf, buf + 10240, ' ' );
        for ( int i = 0; i < 10; ++i ) {
          s.write( buf, 1024*8 );
          EXAM_CHECK_ASYNC( s.good() );
        }
      }

    ~SrvW()
      { }

    void connect( sockstream& )
      { }

    void close()
      { }

  private:
    char buf[10240];
};

class SrvRW
{
  public:
    SrvRW( sockstream& )
      { }

    ~SrvRW()
      { }

    void connect( sockstream& s )
      {
        s.read( buf, 1024*8 );
        EXAM_CHECK_ASYNC( s.good() );
        s.write( buf, 1024*8 ).flush();
        EXAM_CHECK_ASYNC( s.good() );
      }

    void close()
      { }

  private:
    char buf[10240];
};

int EXAM_IMPL(sockios_perf::exchange1)
{
  sockmgr_stream_MP<SrvR> srv( 6480 );
  
  {
    EXAM_REQUIRE( srv.good() && srv.is_open() );

    sockstream s1( "localhost", 6480 );
    char buf[10240];

    fill( buf, buf + 10240, ' ' );

    for ( int i = 0; i < 10; ++i ) {
      s1.write( buf, 1024 * 8 );
      EXAM_CHECK_ASYNC( s1.good() );
    }
  }

  srv.close();
  srv.wait();

  return EXAM_RESULT;
}

int EXAM_IMPL(sockios_perf::exchange2)
{
  sockmgr_stream_MP<SrvW> srv( 6480 );
  
  {
    EXAM_REQUIRE( srv.good() && srv.is_open() );

    sockstream s1( "localhost", 6480 );
    char buf[10240];

    for ( int i = 0; i < 10; ++i ) {
      s1.read( buf, 1024 * 8 );
      EXAM_CHECK_ASYNC( s1.good() );
    }
  }

  srv.close();
  srv.wait();

  return EXAM_RESULT;
}

int EXAM_IMPL(sockios_perf::exchange3)
{
  sockmgr_stream_MP<SrvRW> srv( 6480 );
  
  {
    EXAM_REQUIRE( srv.good() && srv.is_open() );

    sockstream s1( "localhost", 6480 );
    char buf[10240];

    for ( int i = 0; i < 10; ++i ) {
      s1.write( buf, 1024 * 8 ).flush();
      EXAM_CHECK_ASYNC( s1.good() );
      s1.read( buf, 1024 * 8 );
      EXAM_CHECK_ASYNC( s1.good() );
    }
  }

  srv.close();
  srv.wait();

  return EXAM_RESULT;
}

xmt::Thread::ret_t client_thr( void *p )
{
  return 0;
}
