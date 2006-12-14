// -*- C++ -*- Time-stamp: <06/07/08 00:13:31 ptr>

/*
 *
 * Copyright (c) 2002, 2003, 2005
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

#include "ConnectionProcessor.h"
#include <string>
#include "message.h"

using namespace std;

ConnectionProcessor::ConnectionProcessor( std::sockstream& s )
{
  pr_lock.lock();
  BOOST_MESSAGE( "Server seen connection" );

  BOOST_REQUIRE( s.good() );
  pr_lock.unlock();
  // cerr << "Server see connection\n"; // Be silent, avoid interference
  // with Input line prompt
}

void ConnectionProcessor::connect( std::sockstream& s )
{
  pr_lock.lock();
  BOOST_MESSAGE( "Server start connection processing" );

  BOOST_REQUIRE( s.good() );
  pr_lock.unlock();

  string msg;

  getline( s, msg );

  pr_lock.lock();
  BOOST_CHECK_EQUAL( msg, ::message );
  BOOST_REQUIRE( s.good() );
  pr_lock.unlock();

  s << ::message_rsp << endl; // server's response

  pr_lock.lock();
  BOOST_REQUIRE( s.good() );
  BOOST_MESSAGE( "Server stop connection processing" );
  pr_lock.unlock();

  return;
}

void ConnectionProcessor::close()
{
  pr_lock.lock();
  BOOST_MESSAGE( "Server: client close connection" );
  pr_lock.unlock();
}


ConnectionProcessor2::ConnectionProcessor2( std::sockstream& s ) :
    count( 0 )
{
  pr_lock.lock();
  BOOST_MESSAGE( "Server seen connection" );

  BOOST_REQUIRE( s.good() );
  pr_lock.unlock();
  // cerr << "Server see connection\n"; // Be silent, avoid interference
  // with Input line prompt
}

void ConnectionProcessor2::connect( std::sockstream& s )
{
  pr_lock.lock();
  BOOST_MESSAGE( "Server start connection processing" );

  BOOST_REQUIRE( s.good() );
  pr_lock.unlock();

  string msg;

  getline( s, msg );

  switch ( count ) {
    case 0:
      pr_lock.lock();
      BOOST_CHECK_EQUAL( msg, ::message );
      BOOST_REQUIRE( s.good() );
      pr_lock.unlock();

      s << ::message_rsp << endl; // server's response

      pr_lock.lock();
      BOOST_REQUIRE( s.good() );
      BOOST_MESSAGE( "Server stop connection processing" );
      pr_lock.unlock();
      break;
    case 1:
      pr_lock.lock();
      BOOST_CHECK_EQUAL( msg, ::message1 );
      BOOST_REQUIRE( s.good() );
      pr_lock.unlock();

      s << ::message_rsp1 << endl; // server's response

      pr_lock.lock();
      BOOST_REQUIRE( s.good() );
      BOOST_MESSAGE( "Server stop connection processing" );
      pr_lock.unlock();
      break;
    case 2:
      pr_lock.lock();
      BOOST_CHECK_EQUAL( msg, ::message2 );
      BOOST_REQUIRE( s.good() );
      pr_lock.unlock();

      s << ::message_rsp2 << endl; // server's response

      pr_lock.lock();
      BOOST_REQUIRE( s.good() );
      BOOST_MESSAGE( "Server stop connection processing" );
      pr_lock.unlock();
      break;
    default:
      BOOST_ERROR( "Unexpected connection! count not 0, 1, 2!" );
      break;
  }

  ++count;

  return;
}

void ConnectionProcessor2::close()
{
  pr_lock.lock();
  BOOST_MESSAGE( "Server: client close connection" );
  pr_lock.unlock();
}
