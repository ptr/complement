// -*- C++ -*- Time-stamp: <07/07/18 08:32:22 ptr>

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
 * bytes, then flush stream; server read 1 byte in one call
 * to 'connect' and should read the remains byte in another call
 * to 'connect' function.
 * Test show, that server (sockmgr_stream_MP) call 'connect' if some
 * buffered data remains, and do it without signal on poll.
 */

extern int port;

static condition cnd;

class ConnectionProcessor4 // dummy variant
{
  public:
    ConnectionProcessor4( std::sockstream& );

    void connect( std::sockstream& );
    void close();
};

ConnectionProcessor4::ConnectionProcessor4( std::sockstream& s )
{
  EXAM_MESSAGE_ASYNC( "Server seen connection" );

  EXAM_CHECK_ASYNC( s.good() );

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

  EXAM_CHECK_ASYNC( s.good() );

  EXAM_CHECK( c == '0' );
#endif
}

void ConnectionProcessor4::connect( std::sockstream& s )
{
  static int count = 0;

  EXAM_MESSAGE_ASYNC( "Server start connection processing" );

  EXAM_CHECK_ASYNC( s.good() );

  char c = '1';
  s.read( &c, 1 );

  EXAM_CHECK_ASYNC( s.good() );

  if ( count++ == 0 ) {
    EXAM_CHECK_ASYNC( c == '0' );
  } else {
    cnd.set( true );

    EXAM_CHECK_ASYNC( c == '3' );
  }

  // BOOST_REQUIRE( s.good() );
  EXAM_MESSAGE_ASYNC( "Server stop connection processing" );

  return;
}

void ConnectionProcessor4::close()
{
  EXAM_MESSAGE_ASYNC( "Server: client close connection" );
}

int EXAM_IMPL(test_more_bytes_in_socket)
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

  return EXAM_RESULT;
}
