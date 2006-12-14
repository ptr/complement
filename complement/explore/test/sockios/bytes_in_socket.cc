// -*- C++ -*- Time-stamp: <06/10/10 20:35:08 ptr>

/*
 *
 * Copyright (c) 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 2.1
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

#include <boost/test/unit_test.hpp>

using namespace boost::unit_test_framework;

#include <iostream>
#include <list>
#include <mt/xmt.h>

#include <sockios/sockstream>
#include <sockios/sockmgr.h>

using namespace std;
using namespace xmt;

/*
 * Server listen tcp socket; client connect to server and write 2
 * bytes, then flush stream; server read 1 byte in one call
 * to 'connect' and should read the remains byte in another call
 * to 'connect' function.
 * Test show, that server (sockmgr_stream_MP) call 'connect' if some
 * buffered data remains, and do it without signal on poll.
 */

extern int port;
extern xmt::Mutex pr_lock;

static Condition cnd;

class ConnectionProcessor4 // dummy variant
{
  public:
    ConnectionProcessor4( std::sockstream& );

    void connect( std::sockstream& );
    void close();
};

ConnectionProcessor4::ConnectionProcessor4( std::sockstream& s )
{
  pr_lock.lock();
  BOOST_MESSAGE( "Server seen connection" );

  BOOST_REQUIRE( s.good() );
  pr_lock.unlock();

#if 0
  /*
   * Really I have choice: read in constructor, but worry
   * about stream reading or read only in connect...
   */

  // connect( s );
  // cerr << "Server see connection\n"; // Be silent, avoid interference
  // with Input line prompt
  char c = '1';
  s.read( &c, 1 );

  pr_lock.lock();
  BOOST_REQUIRE( s.good() );
  pr_lock.unlock();

  BOOST_CHECK( c == '0' );
#endif
}

void ConnectionProcessor4::connect( std::sockstream& s )
{
  static int count = 0;

  pr_lock.lock();
  BOOST_MESSAGE( "Server start connection processing" );

  BOOST_REQUIRE( s.good() );
  pr_lock.unlock();

  char c = '1';
  s.read( &c, 1 );

  pr_lock.lock();
  BOOST_REQUIRE( s.good() );
  pr_lock.unlock();

  if ( count++ == 0 ) {
    BOOST_CHECK( c == '0' );
  } else {
    cnd.set( true );

    BOOST_CHECK( c == '3' );
  }

  pr_lock.lock();
  // BOOST_REQUIRE( s.good() );
  BOOST_MESSAGE( "Server stop connection processing" );
  pr_lock.unlock();

  return;
}

void ConnectionProcessor4::close()
{
  pr_lock.lock();
  BOOST_MESSAGE( "Server: client close connection" );
  pr_lock.unlock();
}

void test_more_bytes_in_socket()
{
// #ifndef __FIT_NO_POLL
  cnd.set( false );
  sockmgr_stream_MP<ConnectionProcessor4> srv( port ); // start server

  sockstream sock( "localhost", ::port );

  char c[2] = { '0', '3' };
  sock.write( c, 2 );
  sock.flush();

  cnd.try_wait();
  srv.close(); // close server, so we don't wait server termination on next line
  srv.wait(); // Wait for server stop to serve clients connections
// #else
//   BOOST_ERROR( "select-based sockmgr not implemented on this platform" );
// #endif
}
