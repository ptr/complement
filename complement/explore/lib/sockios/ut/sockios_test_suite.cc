// -*- C++ -*- Time-stamp: <07/07/19 00:19:19 ptr>

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

using namespace std;

int EXAM_DECL(test_more_bytes_in_socket);
int EXAM_DECL(test_more_bytes_in_socket2);

int EXAM_IMPL(sockios_test_suite)
{
  exam::test_suite::test_case_type tc[3];

  exam::test_suite t( "libsockios test" );

  // t.flags( t.flags() | exam::base_logger::trace );

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

  t.add( &trivial_sockios_test::listen_iface, trivial_test, "trivial_sockios_test::listen_iface", tc[0] );

  t.add( &trivial_sockios_test::srv_close_connection, trivial_test, "trivial_sockios_test::srv_close_connection", tc[0] );
  t.add( &trivial_sockios_test::shared_socket, trivial_test, "trivial_sockios_test::shared_socket", tc[0] );
  t.add( &trivial_sockios_test::client_close_socket, trivial_test, "trivial_sockios_test::client_close_socket", tc[0] );
  t.add( test_more_bytes_in_socket, "test_more_bytes_in_socket" ); // timeout 5
  t.add( test_more_bytes_in_socket2, "test_more_bytes_in_socket2" ); // timeout 5

  return t.girdle();
}

