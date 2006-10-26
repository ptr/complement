// -*- C++ -*- Time-stamp: <06/10/10 21:00:28 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2005
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

#include "message.h"

#include "ConnectionProcessor.h"
#include "client.h"

void test_client_server_poll()
{
#ifndef __FIT_NO_POLL

  sockmgr_stream_MP<ConnectionProcessor> srv( port ); // start server

  Client::client1(); // start one client

  srv.close(); // close server, so we don't wait server termination on next line
  srv.wait(); // Wait for server stop to serve clients connections
#else
  BOOST_ERROR( "poll-based sockmgr not implemented on this platform" );
#endif
}

void test_client_server_select()
{
#ifndef __FIT_NO_SELECT

  sockmgr_stream_MP_SELECT<ConnectionProcessor> srv( port ); // start server

  Client::client1(); // start one client

  srv.close(); // close server, so we don't wait server termination on next line
  srv.wait(); // Wait for server stop to serve clients connections
#else
  BOOST_ERROR( "select-based sockmgr not implemented on this platform" );
#endif
}

void test_client_server_poll_nonlocal_ack()
{
#ifndef __FIT_NO_POLL
  try {

    // Oh, this trick not work well: hostname may be assigned to 127.0.0.1 too...
    // I need list of interfaces...

    // take primary host IP:
    in_addr hostaddr( findhost( hostname().c_str() ) );
    list<net_iface> ifaces;
    try {
      get_ifaces( ifaces );
    }
    catch ( runtime_error& err ) {
      BOOST_ERROR( err.what() );
    }
    
    list<net_iface>::const_iterator i;
    for ( i = ifaces.begin(); i != ifaces.end(); ++i ) {
      if ( i->name == "eth0" ) {
        hostaddr = i->addr.inet.sin_addr;
        break;
      }
    }
    BOOST_CHECK( i != ifaces.end() );

    // server not listen localhost, but listen ext interface:
    sockmgr_stream_MP<ConnectionProcessor> srv( hostaddr, port ); // start server

    BOOST_CHECK( srv.is_open() );
    BOOST_CHECK( srv.good() );

    Client::client_nonlocal_ack(); // start one client
    
    srv.close(); // close server, so we don't wait server termination on next line
    srv.wait(); // Wait for server stop to serve clients connections
  }
  catch ( std::domain_error& err ) {
    pr_lock.lock();
    BOOST_ERROR( "host not found by name" );
    pr_lock.unlock();
  }
#else
  BOOST_ERROR( "poll-based sockmgr not implemented on this platform" );
#endif
}

void test_client_server_poll_nonlocal_nac()
{
#ifndef __FIT_NO_POLL
  try {
    // take primary host IP:
    in_addr hostaddr( findhost( hostname().c_str() ) );
    list<net_iface> ifaces;
    try {
      get_ifaces( ifaces );
    }
    catch ( runtime_error& err ) {
      BOOST_ERROR( err.what() );
    }
    
    list<net_iface>::const_iterator i;
    for ( i = ifaces.begin(); i != ifaces.end(); ++i ) {
      if ( i->name == "eth0" ) {
        hostaddr = i->addr.inet.sin_addr;
        break;
      }
    }
    BOOST_CHECK( i != ifaces.end() );

    // server not listen localhost, but listen ext interface:
    sockmgr_stream_MP<ConnectionProcessor> srv( hostaddr, port ); // start server

    Client::client_nonlocal_nac(); // start one client

    srv.close(); // close server, so we don't wait server termination on next line
    srv.wait(); // Wait for server stop to serve clients connections
  }
  catch ( std::domain_error& err ) {
    pr_lock.lock();
    BOOST_ERROR( "host not found by name" );
    pr_lock.unlock();
  }
#else
  BOOST_ERROR( "poll-based sockmgr not implemented on this platform" );
#endif
}

void test_client_server_poll_local_ack()
{
#ifndef __FIT_NO_POLL
  try {
    // server listen localhost (127.0.0.1), but not listen ext interface:
    sockmgr_stream_MP<ConnectionProcessor> srv( 0x7f000001, port ); // start server

    Client::client_local_ack(); // start one client
    
    srv.close(); // close server, so we don't wait server termination on next line
    srv.wait(); // Wait for server stop to serve clients connections
  }
  catch ( std::domain_error& err ) {
    pr_lock.lock();
    BOOST_ERROR( "host not found by name" );
    pr_lock.unlock();
  }
#else
  BOOST_ERROR( "poll-based sockmgr not implemented on this platform" );
#endif
}

void test_client_server_select_nonlocal_ack()
{
#ifndef __FIT_NO_SELECT
  try {
    // take primary host IP:
    in_addr hostaddr( findhost( hostname().c_str() ) );
    list<net_iface> ifaces;
    try {
      get_ifaces( ifaces );
    }
    catch ( runtime_error& err ) {
      BOOST_ERROR( err.what() );
    }
    
    list<net_iface>::const_iterator i;
    for ( i = ifaces.begin(); i != ifaces.end(); ++i ) {
      if ( i->name == "eth0" ) {
        hostaddr = i->addr.inet.sin_addr;
        break;
      }
    }
    BOOST_CHECK( i != ifaces.end() );

    // server not listen localhost, but listen ext interface:
    sockmgr_stream_MP_SELECT<ConnectionProcessor> srv( hostaddr, port ); // start server

    Client::client_nonlocal_ack(); // start one client
    
    srv.close(); // close server, so we don't wait server termination on next line
    srv.wait(); // Wait for server stop to serve clients connections
  }
  catch ( std::domain_error& err ) {
    pr_lock.lock();
    BOOST_ERROR( "host not found by name" );
    pr_lock.unlock();
  }
#else
  BOOST_ERROR( "select-based sockmgr not implemented on this platform" );
#endif
}

void test_client_server_select_nonlocal_nac()
{
#ifndef __FIT_NO_SELECT
  try {
    // take primary host IP:
    in_addr hostaddr( findhost( hostname().c_str() ) );
    list<net_iface> ifaces;
    try {
      get_ifaces( ifaces );
    }
    catch ( runtime_error& err ) {
      BOOST_ERROR( err.what() );
    }
    
    list<net_iface>::const_iterator i;
    for ( i = ifaces.begin(); i != ifaces.end(); ++i ) {
      if ( i->name == "eth0" ) {
        hostaddr = i->addr.inet.sin_addr;
        break;
      }
    }
    BOOST_CHECK( i != ifaces.end() );

    // server not listen localhost, but listen ext interface:
    sockmgr_stream_MP_SELECT<ConnectionProcessor> srv( hostaddr, port ); // start server

    Client::client_nonlocal_nac(); // start one client

    srv.close(); // close server, so we don't wait server termination on next line
    srv.wait(); // Wait for server stop to serve clients connections
  }
  catch ( std::domain_error& err ) {
    pr_lock.lock();
    BOOST_ERROR( "host not found by name" );
    pr_lock.unlock();
  }
#else
  BOOST_ERROR( "select-based sockmgr not implemented on this platform" );
#endif
}

void test_client_server_select_local_ack()
{
#ifndef __FIT_NO_POLL
  try {
    // server listen localhost (127.0.0.1), but not listen ext interface:
    sockmgr_stream_MP_SELECT<ConnectionProcessor> srv( 0x7f000001, port ); // start server

    Client::client_local_ack(); // start one client
    
    srv.close(); // close server, so we don't wait server termination on next line
    srv.wait(); // Wait for server stop to serve clients connections
  }
  catch ( std::domain_error& err ) {
    pr_lock.lock();
    BOOST_ERROR( "host not found by name" );
    pr_lock.unlock();
  }
#else
  BOOST_ERROR( "poll-based sockmgr not implemented on this platform" );
#endif
}

void udp_test_client_server_poll()
{
#ifndef __FIT_NO_POLL

  sockmgr_stream_MP<ConnectionProcessor> srv( port, sock_base::sock_dgram ); // start server

  Client::udp_client1(); // start one client

  srv.close(); // close server, so we don't wait server termination on next line
  srv.wait(); // Wait for server stop to serve clients connections
#else
  BOOST_ERROR( "poll-based sockmgr not implemented on this platform" );
#endif
}


int generator_1()
{
  static int i = 0;

  return i++;
}

#include "client-mw.h"

void test_mass_processing_poll()
{
#ifndef __FIT_NO_POLL
  using namespace test_area;

  pr_lock.lock();
  BOOST_REQUIRE( bin_buff1_size == 0 ); // test integrity of test suite
  BOOST_REQUIRE( bin_buff1 == 0 ); // test integrity of test suite
  pr_lock.unlock();

  bin_buff1_size = 48;
  bin_buff1 = new char [bin_buff1_size];
  pr_lock.lock();
  BOOST_REQUIRE( bin_buff1 != 0 );
  pr_lock.unlock();
  generate_n( bin_buff1, bin_buff1_size, generator_1 );

  ni1 = 10;
  ni2 = 5;

  delete bin_buff1;
  bin_buff1 = 0;
  bin_buff1_size = 0;
#else
  BOOST_ERROR( "poll-based sockmgr not implemented on this platform" );
#endif
}

void test_shared_socket()
{
#ifndef __FIT_NO_POLL
  sockmgr_stream_MP<ConnectionProcessor2> srv( port ); // start server

  Client::client_dup(); // start one client

  srv.close(); // close server, so we don't wait server termination on next line
  srv.wait(); // Wait for server stop to serve clients connections
#else
  BOOST_ERROR( "select-based sockmgr not implemented on this platform" );
#endif
}

#include "client-wc.h"

void test_client_close_socket();
void test_more_bytes_in_socket();
void test_more_bytes_in_socket2();
void test_read0();
void test_read0_srv();

struct sockios_test
{
    void hostname_test();
    void service_test();

    void hostaddr_test1();
    void hostaddr_test2();
    void hostaddr_test3();
};

void sockios_test::hostname_test()
{
  unsigned long local = htonl( 0x7f000001 ); // 127.0.0.1

#ifdef _LITTLE_ENDIAN
  BOOST_CHECK_EQUAL( local, 0x0100007f );
#endif

#ifdef _BIG_ENDIAN
  BOOST_CHECK_EQUAL( local, 0x7f000001 );
#endif

  BOOST_CHECK_EQUAL( hostname( local ), "localhost [127.0.0.1]" );

#ifdef __unix
  char buff[1024];

  gethostname( buff, 1024 );

  BOOST_CHECK_EQUAL( hostname(), buff );
#endif
}

void sockios_test::service_test()
{
#ifdef __unix
  BOOST_CHECK( service( "ftp", "tcp" ) == 21 );
  BOOST_CHECK( service( 7, "udp" ) == "echo" );
#else
  BOOST_ERROR( "requests for service (/etc/services) not implemented on this platform" );
#endif
}

void sockios_test::hostaddr_test1()
{
#ifdef __unix
  in_addr addr = findhost( "localhost" );

# ifdef _LITTLE_ENDIAN
  BOOST_CHECK_EQUAL( addr.s_addr, 0x0100007f );
# endif

# ifdef _BIG_ENDIAN
  BOOST_CHECK_EQUAL( addr.s_addr, 0x7f000001 );
# endif
  
#else
  BOOST_ERROR( "Not implemented" );
#endif
}

void sockios_test::hostaddr_test2()
{
#ifdef __unix
  list<in_addr> haddrs;
  gethostaddr( "localhost", back_inserter(haddrs) );

  bool localhost_found = false;

  for ( list<in_addr>::const_iterator i = haddrs.begin(); i != haddrs.end(); ++i ) {
    if ( i->s_addr == htonl( 0x7f000001 ) ) { // 127.0.0.1
      localhost_found = true;
      break;
    }
  }
  
  BOOST_CHECK( localhost_found == true );
  
#else
  BOOST_ERROR( "Not implemented" );
#endif
}

void sockios_test::hostaddr_test3()
{
#ifdef __unix
  list<sockaddr> haddrs;
  gethostaddr2( "localhost", back_inserter(haddrs) );

  bool localhost_found = false;

  for ( list<sockaddr>::const_iterator i = haddrs.begin(); i != haddrs.end(); ++i ) {
    switch ( i->sa_family ) {
      case PF_INET:
        if ( ((sockaddr_in *)&*i)->sin_addr.s_addr == htonl( 0x7f000001 ) ) {
          localhost_found = true;
        }
        break;
      case PF_INET6:
        if ( ((sockaddr_in6 *)&*i)->sin6_addr.in6_u.u6_addr32[0] == 0 &&
             ((sockaddr_in6 *)&*i)->sin6_addr.in6_u.u6_addr32[1] == 0 && 
             ((sockaddr_in6 *)&*i)->sin6_addr.in6_u.u6_addr32[2] == 0 &&
             ((sockaddr_in6 *)&*i)->sin6_addr.in6_u.u6_addr32[3] == 1 ) {
          localhost_found = true;
        }
        break;
    }
  }
  
  BOOST_CHECK( localhost_found == true );
  
#else
  BOOST_ERROR( "Not implemented" );
#endif
}

struct sockios_test_suite :
    public test_suite
{
    sockios_test_suite();
};

sockios_test_suite::sockios_test_suite() :
    test_suite( "sockios library test suite" )
{
  boost::shared_ptr<sockios_test> instance( new sockios_test() );
  test_case *hostname_tc = BOOST_CLASS_TEST_CASE( &sockios_test::hostname_test, instance );
  test_case *service_tc = BOOST_CLASS_TEST_CASE( &sockios_test::service_test, instance );

  test_case *hostaddr1_tc = BOOST_CLASS_TEST_CASE( &sockios_test::hostaddr_test1, instance );
  test_case *hostaddr2_tc = BOOST_CLASS_TEST_CASE( &sockios_test::hostaddr_test2, instance );
  test_case *hostaddr3_tc = BOOST_CLASS_TEST_CASE( &sockios_test::hostaddr_test3, instance );

  // hostaddr2_tc->depends_on( hostaddr1_tc );

  add( hostname_tc );
  add( service_tc );

  add( hostaddr1_tc );
  add( hostaddr2_tc );
  add( hostaddr3_tc );
}

test_suite *init_unit_test_suite( int argc, char **argv )
{
  test_suite *ts = BOOST_TEST_SUITE( "libsockios test" );

  ts->add( new sockios_test_suite() );

  // ts->add( BOOST_TEST_CASE( &hostname_test ) );
  // ts->add( BOOST_TEST_CASE( &service_test ) );

  // ts->add( BOOST_TEST_CASE( &hostaddr_test1 ) );
  // ts->add( BOOST_TEST_CASE( &hostaddr_test2 ) );
  // ts->add( BOOST_TEST_CASE( &hostaddr_test3 ) );

  ts->add( BOOST_TEST_CASE( &test_client_server_poll ) );
  ts->add( BOOST_TEST_CASE( &test_client_server_select ) );

  ts->add( BOOST_TEST_CASE( &test_client_server_poll_nonlocal_ack ) );
  ts->add( BOOST_TEST_CASE( &test_client_server_poll_nonlocal_nac ) );

  ts->add( BOOST_TEST_CASE( &test_client_server_poll_local_ack ) );

  ts->add( BOOST_TEST_CASE( &test_client_server_select_nonlocal_ack ) );
  ts->add( BOOST_TEST_CASE( &test_client_server_select_nonlocal_nac ) );

  ts->add( BOOST_TEST_CASE( &test_client_server_select_local_ack ) );

  ts->add( BOOST_TEST_CASE( &test_mass_processing_poll ) );

  ts->add( BOOST_TEST_CASE( &srv_close_connection_test ) );

  // ts->add( BOOST_TEST_CASE( &udp_test_client_server_poll ) );

  ts->add( BOOST_TEST_CASE( &test_shared_socket ) );

  ts->add( BOOST_TEST_CASE( &test_client_close_socket ) );
  ts->add( BOOST_TEST_CASE( &test_more_bytes_in_socket ), 0, 5 );
  ts->add( BOOST_TEST_CASE( &test_more_bytes_in_socket2 ), 0, 5 );
  ts->add( BOOST_TEST_CASE( &test_read0 ), 0, 7 );
  ts->add( BOOST_TEST_CASE( &test_read0_srv ) );

  return ts;
}
