// -*- C++ -*- Time-stamp: <07/07/18 10:23:40 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2005-2007
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "sockios_test_suite.h"
#include "sockios_test.h"

#include <exam/suite.h>

#include <iostream>
#include <list>
#include <mt/xmt.h>

#include <sockios/sockstream>
#include <sockios/sockmgr.h>

#include "message.h"

#include "ConnectionProcessor.h"

#include "client-wc.h"

using namespace std;

int generator_1()
{
  static int i = 0;

  return i++;
}

int EXAM_IMPL(test_shared_socket)
{
#ifndef __FIT_NO_POLL
  sockmgr_stream_MP<ConnectionProcessor2> srv( port ); // start server

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

int EXAM_DECL(test_client_close_socket);
int EXAM_DECL(test_more_bytes_in_socket);
int EXAM_DECL(test_more_bytes_in_socket2);

int EXAM_IMPL(sockios_test_suite)
{
  exam::test_suite::test_case_type tc[3];

  exam::test_suite t( "libsockios test" );

  trivial_sockios_test trivial_test;

  tc[0] = t.add( &trivial_sockios_test::simple, trivial_test, "trivial_sockios_test::simple" );

  names_sockios_test names_test;

  t.add( &names_sockios_test::hostname_test, names_test, "names_sockios_test::hostname_test" );
  t.add( &names_sockios_test::service_test, names_test, "names_sockios_test::service_test" );
  t.add( &names_sockios_test::hostaddr_test1, names_test, "names_sockios_test::hostaddr_test1" );
  t.add( &names_sockios_test::hostaddr_test2, names_test, "names_sockios_test::hostaddr_test2" );
  t.add( &names_sockios_test::hostaddr_test3, names_test, "names_sockios_test::hostaddr_test3" );

  sockios_test test;

  t.add( &sockios_test::long_msg, test, "sockios_test::long_msg",
         tc[1] = t.add( &sockios_test::ctor_dtor, test, "sockios_test::ctor_dtor", tc[0] ) );

  t.add( &sockios_test::read0, test, "sockios_test::read0", // timeout 5
         tc[2] = t.add( &sockios_test::sigpipe, test, "sockios_test::sigpipe", tc, tc + 2 ) );
  t.add( &sockios_test::read0_srv, test, "sockios_test::read0_srv", tc[2] );
  t.add( &sockios_test::long_block_read, test, "sockios_test::long_block_read", tc[0] );

  // Old tests

  t.add( &trivial_sockios_test::listen_iface, trivial_test, "listen_iface", tc[0] );

  t.add( srv_close_connection_test, "srv_close_connection_test" );
  t.add( test_shared_socket, "test_shared_socket" );
  t.add( test_client_close_socket, "test_client_close_socket" );
  t.add( test_more_bytes_in_socket, "test_more_bytes_in_socket" ); // timeout 5
  t.add( test_more_bytes_in_socket2, "test_more_bytes_in_socket2" ); // timeout 5

  return t.girdle();
}

