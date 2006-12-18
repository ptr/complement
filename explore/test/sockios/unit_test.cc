// -*- C++ -*- Time-stamp: <06/12/18 16:41:59 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2005, 2006
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <boost/test/unit_test.hpp>

using namespace boost::unit_test_framework;

#include "sockios_test_suite.h"

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
  // ts->add( BOOST_TEST_CASE( &test_client_server_select ) );

  ts->add( BOOST_TEST_CASE( &test_client_server_poll_nonlocal_ack ) );
  ts->add( BOOST_TEST_CASE( &test_client_server_poll_nonlocal_nac ) );

  ts->add( BOOST_TEST_CASE( &test_client_server_poll_local_ack ) );

  // ts->add( BOOST_TEST_CASE( &test_client_server_select_nonlocal_ack ) );
  // ts->add( BOOST_TEST_CASE( &test_client_server_select_nonlocal_nac ) );

  // ts->add( BOOST_TEST_CASE( &test_client_server_select_local_ack ) );

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
