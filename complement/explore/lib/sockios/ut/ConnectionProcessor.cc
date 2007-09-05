// -*- C++ -*- Time-stamp: <07/09/05 00:39:10 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2005, 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <exam/suite.h>

#include "ConnectionProcessor.h"
#include <string>
#include "message.h"

#include <sockios/sockmgr.h>

#include <mt/xmt.h>

using namespace std;
using namespace xmt;

ConnectionProcessor::ConnectionProcessor( std::sockstream& s )
{
  EXAM_MESSAGE_ASYNC( "Server seen connection" );

  EXAM_CHECK_ASYNC( s.good() );
  // cerr << "Server see connection\n"; // Be silent, avoid interference
  // with Input line prompt
}

void ConnectionProcessor::connect( std::sockstream& s )
{
  EXAM_MESSAGE_ASYNC( "Server start connection processing" );

  EXAM_CHECK_ASYNC( s.good() );

  string msg;

  getline( s, msg );

  EXAM_CHECK_ASYNC( msg == ::message );
  EXAM_CHECK_ASYNC( s.good() );

  s << ::message_rsp << endl; // server's response

  EXAM_CHECK_ASYNC( s.good() );
  EXAM_MESSAGE_ASYNC( "Server stop connection processing" );

  return;
}

void ConnectionProcessor::close()
{
  EXAM_MESSAGE_ASYNC( "Server: client close connection" );
}

// ******************

trivial_sockios_test::trivial_sockios_test() :
    hostaddr( findhost( hostname().c_str() ) ) // take primary host IP
{
  // Oh, this trick not work well: hostname may be assigned to 127.0.0.1 too...
  // I need list of interfaces...

  list<net_iface> ifaces;
  try {
    get_ifaces( ifaces );
  }
  catch ( runtime_error& err ) {
    EXAM_ERROR_ASYNC( err.what() );
  }

  list<net_iface>::const_iterator i;
  for ( i = ifaces.begin(); i != ifaces.end(); ++i ) {
    if ( i->name == "eth0" ) {
      hostaddr = i->addr.inet.sin_addr;
      // hostaddr = i->addr.any; // .inet.sin_addr;
      break;
    }
  }
  EXAM_CHECK_ASYNC( i != ifaces.end() );

  for ( i = ifaces.begin(); i != ifaces.end(); ++i ) {
    if ( i->name == "lo" ) {
      localaddr = i->addr.inet.sin_addr;
      // hostaddr = i->addr.any; // .inet.sin_addr;
      break;
    }
  }
  EXAM_CHECK_ASYNC( i != ifaces.end() );
}

int EXAM_IMPL(trivial_sockios_test::simple)
{
#ifndef __FIT_NO_POLL

  std::sockmgr_stream_MP<ConnectionProcessor> srv( port ); // start server

  {
    EXAM_MESSAGE( "Client start" );
    std::sockstream sock( "localhost", ::port );
    string srv_line;

    sock << ::message << endl;

    EXAM_CHECK( sock.good() );

    // sock.clear();
    getline( sock, srv_line );

    EXAM_CHECK( sock.good() );

    EXAM_CHECK( srv_line == ::message_rsp );

    EXAM_MESSAGE( "Client close connection (client's end of life)" );
    // sock.close(); // no needs, that will done in sock destructor
  }

  {
    std::sockstream sock( "127.0.0.1", ::port );
    string srv_line;

    sock << ::message << endl;

    EXAM_CHECK( sock.good() );

    // sock.clear();
    getline( sock, srv_line );

    EXAM_CHECK( sock.good() );

    EXAM_CHECK( srv_line == ::message_rsp );
  }

  srv.close(); // close server, so we don't wait server termination on next line
  srv.wait(); // Wait for server stop to serve clients connections
#else
  EXAM_ERROR( "poll-based sockmgr not implemented on this platform" );
#endif

  return EXAM_RESULT;
}

// ******************

int EXAM_IMPL(trivial_sockios_test::listen_iface)
{
#ifndef __FIT_NO_POLL
  try {
    // server not listen localhost, but listen ext interface:
    sockmgr_stream_MP<ConnectionProcessor> srv( hostaddr, port ); // start server

    EXAM_CHECK( srv.is_open() );
    EXAM_CHECK( srv.good() );

    {
      EXAM_MESSAGE( "Client start" );

      // std::sockstream sock( hostname(hostaddr.s_addr).c_str(), ::port );
      // std::sockstream sock( (*(sockaddr_in *)&hostaddr).sin_addr, ::port );
      std::sockstream sock( hostaddr, ::port );
      string srv_line;

      EXAM_CHECK( sock.good() );

      sock << ::message << endl;

      EXAM_CHECK( sock.good() );

      // sock.clear();
      getline( sock, srv_line );

      EXAM_CHECK( sock.good() );

      EXAM_CHECK( srv_line == ::message_rsp );

      EXAM_MESSAGE( "Client close connection (client's end of life)" );
      // sock.close(); // no needs, that will done in sock destructor
    }

    {
      std::sockstream sock( localaddr, ::port );

      EXAM_CHECK( !sock.is_open() );
    }

    srv.close(); // close server, so we don't wait server termination on next line
    srv.wait(); // Wait for server stop to serve clients connections
  }
  catch ( std::domain_error& err ) {
    EXAM_ERROR( "host not found by name" );
  }
#else
  EXAM_ERROR( "poll-based sockmgr not implemented on this platform" );
#endif

  return EXAM_RESULT;
}

// ******************

ConnectionProcessor2::ConnectionProcessor2( std::sockstream& s ) :
    count( 0 )
{
  EXAM_MESSAGE_ASYNC( "Server seen connection" );

  EXAM_CHECK_ASYNC( s.good() );
  // cerr << "Server see connection\n"; // Be silent, avoid interference
  // with Input line prompt
}

void ConnectionProcessor2::connect( std::sockstream& s )
{
  EXAM_MESSAGE_ASYNC( "Server start connection processing" );

  EXAM_CHECK_ASYNC( s.good() );

  string msg;

  getline( s, msg );

  switch ( count ) {
    case 0:
      EXAM_CHECK_ASYNC( msg == ::message );
      EXAM_CHECK_ASYNC( s.good() );

      s << ::message_rsp << endl; // server's response

      EXAM_CHECK_ASYNC( s.good() );
      EXAM_MESSAGE_ASYNC( "Server stop connection processing" );
      break;
    case 1:
      EXAM_CHECK_ASYNC( msg == ::message1 );
      EXAM_CHECK_ASYNC( s.good() );

      s << ::message_rsp1 << endl; // server's response

      EXAM_CHECK_ASYNC( s.good() );
      EXAM_MESSAGE_ASYNC( "Server stop connection processing" );
      break;
    case 2:
      EXAM_CHECK_ASYNC( msg == ::message2 );
      EXAM_CHECK_ASYNC( s.good() );

      s << ::message_rsp2 << endl; // server's response

      EXAM_CHECK_ASYNC( s.good() );
      EXAM_MESSAGE_ASYNC( "Server stop connection processing" );
      break;
    default:
      EXAM_ERROR_ASYNC( "Unexpected connection! count not 0, 1, 2!" );
      break;
  }

  ++count;

  return;
}

void ConnectionProcessor2::close()
{
  EXAM_MESSAGE_ASYNC( "Server: client close connection" );
}

int EXAM_IMPL(trivial_sockios_test::shared_socket)
{
#ifndef __FIT_NO_POLL
  sockmgr_stream_MP<ConnectionProcessor2> srv( port ); // start server

  EXAM_CHECK( srv.is_open() );
  EXAM_CHECK( srv.good() );

  {
    EXAM_MESSAGE( "Client start" );
    std::sockstream sock( "localhost", ::port );
    string srv_line;

    sock << ::message << endl;

    EXAM_CHECK( sock.good() );

    // sock.clear();
    getline( sock, srv_line );

    EXAM_CHECK( sock.good() );

    EXAM_CHECK( srv_line == ::message_rsp );

    EXAM_MESSAGE( "Client close connection (client's end of life)" );

    {
      std::sockstream sock2;
      sock2.attach( sock.rdbuf()->fd() );

      sock2 << ::message1 << endl;

      EXAM_CHECK( sock.good() );
      EXAM_CHECK( sock2.good() );

      srv_line.clear();
      getline( sock2, srv_line );

      EXAM_CHECK( sock.good() );
      EXAM_CHECK( sock2.good() );

      EXAM_CHECK( srv_line == ::message_rsp1 );

      EXAM_MESSAGE( "Subclient close connection (subclient's end of life)" );
    }

    sock << ::message2 << endl;

    EXAM_CHECK( sock.good() );

    // sock.clear();
    srv_line.clear();
    getline( sock, srv_line );

    EXAM_CHECK( sock.good() );

    EXAM_CHECK( srv_line == ::message_rsp2 );

    EXAM_MESSAGE( "Client close connection (client's end of life)" );

    // sock.close(); // no needs, that will done in sock destructor
  }

  srv.close(); // close server, so we don't wait server termination on next line
  srv.wait(); // Wait for server stop to serve clients connections
#else
  EXAM_ERROR( "select-based sockmgr not implemented on this platform" );
#endif

  return EXAM_RESULT;
}

/* ******************
 *
 * Check correct processing of case when server close connection.
 * Suspicious processing with FreeBSD and OpenBSD servers.
 *
 */
static condition cnd_close;

class Srv // 
{
  public:
    Srv( std::sockstream& );

    void connect( std::sockstream& );
    void close();
};

Srv::Srv( std::sockstream& s )
{
  s << "hello" << endl;

  // xmt::delay( xmt::timespec( 1, 0 ) );

  s.close();
  // ::shutdown( s.rdbuf()->fd(), 2 );
  cnd_close.set( true );
}

void Srv::connect( std::sockstream& )
{
}

void Srv::close()
{
}

#ifndef __FIT_NO_POLL
typedef sockmgr_stream_MP<Srv> srv_type;
#elif defined(__FIT_NO_SELECT)
typedef sockmgr_stream_MP_SELECT<Srv> srv_type;
#else
# error Either poll or select should be present!
#endif

static srv_type *srv_p;
condition cnd;

Thread::ret_t server_proc( void * )
{
  unsigned rt = 0;

  cnd.set( false );
  srv_type srv( port ); // start server

  ::srv_p = &srv;

  if ( !srv.is_open() || !srv.good() ) {
    ++rt;
  }

  cnd.set( true );

  srv.wait();

  return reinterpret_cast<Thread::ret_t>(rt);
}

Thread::ret_t client_proc( void * )
{
  unsigned rt = 0;

  cnd.try_wait();

  EXAM_MESSAGE_ASYNC( "Client start" );
  std::sockstream sock( "localhost", ::port );

  string buf;

  getline( sock, buf );

  if ( !sock.is_open() || !sock.good() ) {
    ++rt;
  }

  EXAM_CHECK_ASYNC( buf == "hello" );

  // xmt::delay( xmt::timespec( 5, 0 ) );

  // sock << 'a' << endl;

  /*
    read required here, due to we can see FIN packet only on read,
    and no other solution! (another solution is nonblock sockets or
    aio, but this is another story)
  */
  cnd_close.try_wait();

  char a;
  sock.read( &a, 1 );

  EXAM_CHECK_ASYNC( !sock.good() );

  srv_p->close();

  EXAM_MESSAGE_ASYNC( "Client end" );

  return reinterpret_cast<Thread::ret_t>(rt);
}

int EXAM_IMPL(trivial_sockios_test::srv_close_connection)
{
  Thread srv( server_proc );
  cnd_close.set( false );
  Thread client( client_proc );

  EXAM_CHECK( client.join() == 0 );
  EXAM_CHECK( srv.join() == 0 );

  return EXAM_RESULT;
}

/*
 * Server listen tcp socket; client connect to server and try to read
 * what server write to socket; server don't write anything, but we
 * try to close connection (close socket on client's side, but from
 * differrent thread from reading socket).
 * I suspect that closing socket on client side don't lead to break down
 * through read call.
 */

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

std::sockstream *psock = 0;

Thread::ret_t thread_entry_call( void * )
{
  cnd.set( true );

  EXAM_MESSAGE_ASYNC( "Client start" );

  EXAM_CHECK_ASYNC( psock->good() );

  char c = '0';
  psock->read( &c, 1 );
  EXAM_CHECK_ASYNC( c == '1' );
  cnd_close.set( true );
  psock->read( &c, 1 );

  return 0;
}

int EXAM_IMPL(trivial_sockios_test::client_close_socket)
{
#ifndef __FIT_NO_POLL
  sockmgr_stream_MP<ConnectionProcessor3> srv( port ); // start server

  cnd.set( false );
  cnd_close.set( false );

  // open client's socket _before_ thread launch to demonstrate problem with
  // socket close (close socket's descriptor in one thread don't lead to real
  // shutdown events if socket in use in another thread)
  psock = new std::sockstream( "localhost", ::port );
  xmt::Thread thr( thread_entry_call );
  cnd.try_wait();
  // close socket; you may expect that sock.read break down, but this
  // will not happens: this thread has one copy of (psock) file descriptor,
  // thread_entry_call has another; we close only one
  cnd_close.try_wait();
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
