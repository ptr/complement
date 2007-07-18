// -*- C++ -*- Time-stamp: <07/07/11 21:34:53 ptr>

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
 * Server listen tcp socket; client connect to server and try to read
 * what server write to socket; server don't write anything, but we
 * try to close connection (close socket on client's side, but from
 * differrent thread from reading socket).
 * I suspect that closing socket on client side don't lead to break down
 * through read call.
 */

extern int port;

class ConnectionProcessor3 // dummy variant
{
  public:
    ConnectionProcessor3( std::sockstream& );

    void connect( std::sockstream& );
    void close();
};

ConnectionProcessor3::ConnectionProcessor3( std::sockstream& s )
{
  EXAM_MESSAGE_ASYNC( "Server seen connection" );

  EXAM_CHECK_ASYNC( s.good() );
  connect( s );
  // cerr << "Server see connection\n"; // Be silent, avoid interference
  // with Input line prompt
}

condition cnd2;

void ConnectionProcessor3::connect( std::sockstream& s )
{
  EXAM_MESSAGE_ASYNC( "Server start connection processing" );

  EXAM_CHECK_ASYNC( s.good() );

  // string msg;

  // getline( s, msg );
  char c = '1';
  s.write( &c, 1 );
  s.flush();
  // cnd2.set( true );
  // EXAM_CHECK_EQUAL( msg, ::message );
  EXAM_CHECK_ASYNC( s.good() );

  // s << ::message_rsp << endl; // server's response

  // BOOST_REQUIRE( s.good() );
  EXAM_MESSAGE_ASYNC( "Server stop connection processing" );

  return;
}

void ConnectionProcessor3::close()
{
  EXAM_MESSAGE_ASYNC( "Server: client close connection" );
}

condition cnd1;
// Condition cnd2;
std::sockstream *psock = 0;

Thread::ret_code thread_entry_call( void * )
{
  Thread::ret_code rt;
  rt.iword = 0;

  cnd1.set( true );

  EXAM_MESSAGE_ASYNC( "Client start" );

  EXAM_CHECK_ASYNC( psock->good() );

  char c = '0';
  psock->read( &c, 1 );
  EXAM_CHECK_ASYNC( c == '1' );
  cnd2.set( true );
  psock->read( &c, 1 );

  return rt;
}

int EXAM_IMPL(test_client_close_socket)
{
#ifndef __FIT_NO_POLL
  sockmgr_stream_MP<ConnectionProcessor3> srv( port ); // start server

  cnd1.set( false );
  cnd2.set( false );

  // open client's socket _before_ thread launch to demonstrate problem with
  // socket close (close socket's descriptor in one thread don't lead to real
  // shutdown events if socket in use in another thread)
  psock = new std::sockstream( "localhost", ::port );
  xmt::Thread thr( thread_entry_call );
  cnd1.try_wait();
  // close socket; you may expect that sock.read break down, but this
  // will not happens: this thread has one copy of (psock) file descriptor,
  // thread_entry_call has another; we close only one
  cnd2.try_wait();
  // but call shutdown is what you want here:
  psock->rdbuf()->shutdown( sock_base::stop_in | sock_base::stop_out );
  psock->close();
  thr.join();
  delete psock;

  srv.close(); // close server, so we don't wait server termination on next line
  srv.wait(); // Wait for server stop to serve clients connections
#else
  EXAM_ERROR( "select-based sockmgr not implemented on this platform" );
#endif

  return EXAM_RESULT;
}
