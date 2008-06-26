// -*- C++ -*- Time-stamp: <07/07/11 21:36:14 ptr>

/*
 *
 * Copyright (c) 2006, 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <exam/suite.h>

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
  EXAM_MESSAGE_ASYNC( "Server seen connection" );

  EXAM_CHECK_ASYNC( s.good() );

  // connect( s );
  // cerr << "Server see connection\n"; // Be silent, avoid interference
  // with Input line prompt
  char c = '1';
  s.read( &c, 1 );

  EXAM_CHECK_ASYNC( s.good() );

  EXAM_CHECK_ASYNC( c == '0' );
}

void ConnectionProcessor7::connect( std::sockstream& s )
{
  EXAM_MESSAGE_ASYNC( "Server start connection processing" );

  EXAM_CHECK_ASYNC( s.good() );

  char c = '1';
  s.read( &c, 1 );

  EXAM_CHECK_ASYNC( s.good() );

  cnd.set( true );

  EXAM_CHECK_ASYNC( c == '3' );

  // BOOST_REQUIRE( s.good() );
  EXAM_MESSAGE_ASYNC( "Server stop connection processing" );

  return;
}

void ConnectionProcessor7::close()
{
  EXAM_MESSAGE_ASYNC( "Server: client close connection" );
}

int EXAM_IMPL(test_more_bytes_in_socket2)
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

  return EXAM_RESULT;
}
