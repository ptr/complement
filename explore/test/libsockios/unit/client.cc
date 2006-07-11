// -*- C++ -*- Time-stamp: <06/07/11 12:56:11 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2005, 2006
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

#include <boost/test/test_tools.hpp>

#include <string>
#include <sockios/sockstream>
#include <iostream>
#include <iomanip>
#include <list>
#include <mt/xmt.h>

#include "client.h"
#include "message.h"

using namespace std;
using namespace xmt;

void Client::client1()
{
  pr_lock.lock();
  BOOST_MESSAGE( "Client start" );
  pr_lock.unlock();
  std::sockstream sock( "localhost", ::port );
  string srv_line;

  sock << ::message << endl;

  pr_lock.lock();
  BOOST_CHECK( sock.good() );
  pr_lock.unlock();

  // sock.clear();
  getline( sock, srv_line );

  pr_lock.lock();
  BOOST_CHECK( sock.good() );

  BOOST_CHECK_EQUAL( srv_line, ::message_rsp );

  BOOST_MESSAGE( "Client close connection (client's end of life)" );
  pr_lock.unlock();
  // sock.close(); // no needs, that will done in sock destructor
}

void Client::client_nonlocal_ack()
{
  pr_lock.lock();
  BOOST_MESSAGE( "Client start" );
  pr_lock.unlock();

  // in_addr hostaddr( findhost( hostname().c_str() ) );
  sockaddr hostaddr;

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
      hostaddr = i->addr.any; // .inet.sin_addr;
      break;
    }
  }
  BOOST_CHECK( i != ifaces.end() );

  // std::sockstream sock( hostname(hostaddr.s_addr).c_str(), ::port );
  std::sockstream sock( (*(sockaddr_in *)&hostaddr).sin_addr, ::port );
  string srv_line;

  pr_lock.lock();
  BOOST_CHECK( sock.good() );
  pr_lock.unlock();

  sock << ::message << endl;

  pr_lock.lock();
  BOOST_CHECK( sock.good() );
  pr_lock.unlock();

  // sock.clear();
  getline( sock, srv_line );

  pr_lock.lock();
  BOOST_CHECK( sock.good() );

  BOOST_CHECK_EQUAL( srv_line, ::message_rsp );

  BOOST_MESSAGE( "Client close connection (client's end of life)" );
  pr_lock.unlock();
  // sock.close(); // no needs, that will done in sock destructor
}

void Client::client_nonlocal_nac()
{
  pr_lock.lock();
  BOOST_MESSAGE( "Client start" );
  pr_lock.unlock();
  // server listen only primary host's IP, not 127.0.0.1
  std::sockstream sock( /* "localhost" */ "127.0.0.1", ::port );

  pr_lock.lock();
  BOOST_CHECK( !sock.is_open() );
  pr_lock.unlock();
}

void Client::client_local_ack()
{
  pr_lock.lock();
  BOOST_MESSAGE( "Client start" );
  pr_lock.unlock();

  std::sockstream sock( "127.0.0.1", ::port );
  string srv_line;

  sock << ::message << endl;

  pr_lock.lock();
  BOOST_CHECK( sock.good() );
  pr_lock.unlock();

  // sock.clear();
  getline( sock, srv_line );

  pr_lock.lock();
  BOOST_CHECK( sock.good() );

  BOOST_CHECK_EQUAL( srv_line, ::message_rsp );

  BOOST_MESSAGE( "Client close connection (client's end of life)" );
  pr_lock.unlock();
  // sock.close(); // no needs, that will done in sock destructor
}

void Client::udp_client1()
{
  pr_lock.lock();
  BOOST_MESSAGE( "Client start" );
  pr_lock.unlock();
  std::sockstream sock( "localhost", ::port, sock_base::sock_dgram );
  string srv_line;

  sock << ::message << endl;

  pr_lock.lock();
  BOOST_CHECK( sock.good() );
  pr_lock.unlock();

  // sock.clear();
  getline( sock, srv_line );

  pr_lock.lock();
  BOOST_CHECK( sock.good() );

  BOOST_CHECK_EQUAL( srv_line, ::message_rsp );

  BOOST_MESSAGE( "Client close connection (client's end of life)" );
  pr_lock.unlock();
  // sock.close(); // no needs, that will done in sock destructor
}

void Client::client_dup()
{
  pr_lock.lock();
  BOOST_MESSAGE( "Client start" );
  pr_lock.unlock();
  std::sockstream sock( "localhost", ::port );
  string srv_line;

  sock << ::message << endl;

  pr_lock.lock();
  BOOST_CHECK( sock.good() );
  pr_lock.unlock();

  // sock.clear();
  getline( sock, srv_line );

  pr_lock.lock();
  BOOST_CHECK( sock.good() );

  BOOST_CHECK_EQUAL( srv_line, ::message_rsp );

  BOOST_MESSAGE( "Client close connection (client's end of life)" );
  pr_lock.unlock();

  {
    std::sockstream sock2;
    sock2.attach( sock.rdbuf()->fd() );

    sock2 << ::message1 << endl;

    pr_lock.lock();
    BOOST_CHECK( sock.good() );
    BOOST_CHECK( sock2.good() );
    pr_lock.unlock();

    srv_line.clear();
    getline( sock2, srv_line );

    pr_lock.lock();
    BOOST_CHECK( sock.good() );
    BOOST_CHECK( sock2.good() );

    BOOST_CHECK_EQUAL( srv_line, ::message_rsp1 );

    BOOST_MESSAGE( "Subclient close connection (subclient's end of life)" );
    pr_lock.unlock();
  }

  sock << ::message2 << endl;

  pr_lock.lock();
  BOOST_CHECK( sock.good() );
  pr_lock.unlock();

  // sock.clear();
  srv_line.clear();
  getline( sock, srv_line );

  pr_lock.lock();
  BOOST_CHECK( sock.good() );

  BOOST_CHECK_EQUAL( srv_line, ::message_rsp2 );

  BOOST_MESSAGE( "Client close connection (client's end of life)" );
  pr_lock.unlock();

  // sock.close(); // no needs, that will done in sock destructor
}
