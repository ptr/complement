// -*- C++ -*- Time-stamp: <07/07/18 08:46:23 ptr>

/*
 * Copyright (c) 2004, 2006, 2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 3.0
 *
 */

#include <exam/suite.h>

#include <string>
#include <sockios/sockstream>
#include <sockios/sockmgr.h>
#include <iostream>
#include <iomanip>
#include <mt/xmt.h>

#include "client-wc.h"
#include "message.h"

using namespace std;
using namespace xmt;

/*
  Check correct processing of case when server close connection.
  Suspicious processing with FreeBSD and OpenBSD servers.

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

Thread::ret_code server_proc( void * )
{
  cnd.set( false );
  srv_type srv( port ); // start server

  ::srv_p = &srv;

  cnd.set( true );

  srv.wait();

  Thread::ret_code rt;
  rt.iword = 0;

  return rt;
}

Thread::ret_code client_proc( void * )
{
  Thread::ret_code rt;
  rt.iword = 0;

  cnd.try_wait();

  EXAM_MESSAGE_ASYNC( "Client start" );
  std::sockstream sock( "localhost", ::port );

  string buf;

  getline( sock, buf );

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

  return rt;
}

int EXAM_IMPL(srv_close_connection_test)
{
  Thread srv( server_proc );
  cnd_close.set( false );
  Thread client( client_proc );

  client.join();
  srv.join();

  return EXAM_RESULT;
}
