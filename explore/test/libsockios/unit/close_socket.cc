// -*- C++ -*- Time-stamp: <06/07/08 00:26:36 ptr>

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
 * Server listen tcp socket; client connect to server and try to read
 * what server write to socket; server don't write anything, but we
 * try to close connection (close socket on client's side, but from
 * differrent thread from reading socket).
 * I suspect that closing socket on client side don't lead to break down
 * through read call.
 */

extern int port;
extern xmt::Mutex pr_lock;

class ConnectionProcessor3 // dummy variant
{
  public:
    ConnectionProcessor3( std::sockstream& );

    void connect( std::sockstream& );
    void close();
};

ConnectionProcessor3::ConnectionProcessor3( std::sockstream& s )
{
  pr_lock.lock();
  BOOST_MESSAGE( "Server seen connection" );

  BOOST_REQUIRE( s.good() );
  pr_lock.unlock();
  connect( s );
  // cerr << "Server see connection\n"; // Be silent, avoid interference
  // with Input line prompt
}

Condition cnd2;

void ConnectionProcessor3::connect( std::sockstream& s )
{
  pr_lock.lock();
  BOOST_MESSAGE( "Server start connection processing" );

  BOOST_REQUIRE( s.good() );
  pr_lock.unlock();

  // string msg;

  // getline( s, msg );
  char c = '1';
  s.write( &c, 1 );
  s.flush();
  // cnd2.set( true );
  pr_lock.lock();
  // BOOST_CHECK_EQUAL( msg, ::message );
  BOOST_REQUIRE( s.good() );
  pr_lock.unlock();

  // s << ::message_rsp << endl; // server's response

  pr_lock.lock();
  // BOOST_REQUIRE( s.good() );
  BOOST_MESSAGE( "Server stop connection processing" );
  pr_lock.unlock();

  return;
}

void ConnectionProcessor3::close()
{
  pr_lock.lock();
  BOOST_MESSAGE( "Server: client close connection" );
  pr_lock.unlock();
}

Condition cnd1;
// Condition cnd2;
std::sockstream *psock = 0;

int thread_entry_call( void * )
{
  cnd1.set( true );

  pr_lock.lock();
  BOOST_MESSAGE( "Client start" );
  pr_lock.unlock();

  BOOST_REQUIRE( psock->good() );

  char c = '0';
  psock->read( &c, 1 );
  BOOST_CHECK( c == '1' );
  cnd2.set( true );
  psock->read( &c, 1 );

  return 0;
}

void test_client_close_socket()
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
  BOOST_ERROR( "select-based sockmgr not implemented on this platform" );
#endif
}
