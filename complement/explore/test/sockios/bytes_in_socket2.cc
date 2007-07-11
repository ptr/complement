// -*- C++ -*- Time-stamp: <07/07/11 21:36:14 ptr>

/*
 *
 * Copyright (c) 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
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
 * bytes, then flush stream; server read 1 byte in ctor
 * and should read the remains byte in call to 'connect' function.
 * Test show, that server (sockmgr_stream_MP) call 'connect' if some
 * buffered data remains, and do it without signal on poll.
 */

extern int port;
extern xmt::mutex pr_lock;

static condition cnd;

class ConnectionProcessor7 // dummy variant
{
  public:
    ConnectionProcessor7( std::sockstream& );

    void connect( std::sockstream& );
    void close();
};

ConnectionProcessor7::ConnectionProcessor7( std::sockstream& s )
{
  pr_lock.lock();
  BOOST_MESSAGE( "Server seen connection" );

  BOOST_REQUIRE( s.good() );
  pr_lock.unlock();

  // connect( s );
  // cerr << "Server see connection\n"; // Be silent, avoid interference
  // with Input line prompt
  char c = '1';
  s.read( &c, 1 );

  pr_lock.lock();
  BOOST_REQUIRE( s.good() );
  pr_lock.unlock();

  BOOST_CHECK( c == '0' );
}

void ConnectionProcessor7::connect( std::sockstream& s )
{
  pr_lock.lock();
  BOOST_MESSAGE( "Server start connection processing" );

  BOOST_REQUIRE( s.good() );
  pr_lock.unlock();

  char c = '1';
  s.read( &c, 1 );

  pr_lock.lock();
  BOOST_REQUIRE( s.good() );
  pr_lock.unlock();

  cnd.set( true );

  BOOST_CHECK( c == '3' );

  pr_lock.lock();
  // BOOST_REQUIRE( s.good() );
  BOOST_MESSAGE( "Server stop connection processing" );
  pr_lock.unlock();

  return;
}

void ConnectionProcessor7::close()
{
  pr_lock.lock();
  BOOST_MESSAGE( "Server: client close connection" );
  pr_lock.unlock();
}

void test_more_bytes_in_socket2()
{
// #ifndef __FIT_NO_POLL
  cnd.set( false );
  sockmgr_stream_MP<ConnectionProcessor7> srv( port ); // start server

  sockstream sock( "localhost", ::port );

  char c[5] = { '0', '3', '4', '5', '6' };
  sock.write( c, 2 );
  sock.flush();

  cnd.try_wait();
  srv.close(); // close server, so we don't wait server termination on next line
  srv.wait(); // Wait for server stop to serve clients connections
// #else
//   BOOST_ERROR( "select-based sockmgr not implemented on this platform" );
// #endif
}
