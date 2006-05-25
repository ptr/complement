// -*- C++ -*- Time-stamp: <03/08/15 19:03:44 ptr>

/*
 *
 * Copyright (c) 2002, 2003
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 1.2
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

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#ident "@(#)$Id$"
#  endif
#endif

#include "ConnectionProcessor.h"
#include <string>

using namespace std;

ConnectionProcessor::ConnectionProcessor( std::sockstream& s )
{
  // BOOST_MESSAGE( "Server seen connection" );

  // BOOST_REQUIRE( s.good() );
  // cerr << "Server see connection\n"; // Be silent, avoid interference
  // with Input line prompt
}

void ConnectionProcessor::connect( std::sockstream& s )
{
  // BOOST_MESSAGE( "Server start connection processing" );

  // BOOST_REQUIRE( s.good() );

  string msg;

  getline( s, msg );

  cerr << msg << endl;
  // BOOST_CHECK_EQUAL( msg, ::message );
  // BOOST_REQUIRE( s.good() );

  // s << ::message_rsp << endl; // server's response

  // BOOST_REQUIRE( s.good() );
  // BOOST_MESSAGE( "Server stop connection processing" );

  return;
}

void ConnectionProcessor::close()
{
  // BOOST_MESSAGE( "Server: client close connection" );
}
