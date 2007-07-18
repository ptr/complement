// -*- C++ -*- Time-stamp: <07/07/18 10:17:29 ptr>

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

using namespace std;

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

  srv.close(); // close server, so we don't wait server termination on next line
  srv.wait(); // Wait for server stop to serve clients connections
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
