// -*- C++ -*- Time-stamp: <03/09/29 15:21:38 ptr>

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id: unit_test.cc,v 1.2 2003/10/06 07:56:08 ptr Exp $"
#  else
#ident "@(#)$Id: unit_test.cc,v 1.2 2003/10/06 07:56:08 ptr Exp $"
#  endif
#endif

#include <boost/test/unit_test.hpp>

using namespace boost::unit_test_framework;
using namespace std;

#include <iostream>
#include <sstream>

#include "stategraph.h"


enum vertex {
  NotConnected,
  Connected,
  Greeting,
  NVertices
};

enum edge {
  _connect_,
  _disconnect_,
  _QUIT_,
  _HELO_,
  _NCommands_
};

void girdle_test_1()
{
  stringstream s;

  StateGraph g( NVertices, s );

  g.edge( NotConnected, Connected, 5, _connect_ );
  g.edge( Connected, Greeting, 1, _HELO_ );
  g.edge( Greeting, NotConnected, 5, _QUIT_ );

  g[_connect_] << "1.";
  g[_disconnect_] << "2.";
  g[_QUIT_] << "3.";
  g[_HELO_] << "4.";

  g.girdle( NotConnected );

  BOOST_REQUIRE( s.str() == "1.4.3." );
}

void girdle_test_2()
{
  stringstream s;

  StateGraph g( NVertices, s );

  g.edge( NotConnected, Connected, 5, _connect_ );
  g.edge( Connected, Greeting, 1, _HELO_ );
  g.edge( Greeting, NotConnected, 5, _QUIT_ );
  g.edge( Greeting, NotConnected, 10, _disconnect_ );
  g.edge( Connected, NotConnected, 10, _disconnect_ );

  g[_connect_] << "1.";
  g[_disconnect_] << "2.";
  g[_QUIT_] << "3.";
  g[_HELO_] << "4.";

  g.girdle( NotConnected );

  BOOST_REQUIRE( s.str() == "1.4.3.1.2.1.4.2." );
  // cout <<  s.str() << endl;
}

test_suite *init_unit_test_suite( int argc, char **argv )
{
  test_suite *ts = BOOST_TEST_SUITE( "StateGraph test" );

  ts->add( BOOST_TEST_CASE( &girdle_test_1 ) );
  ts->add( BOOST_TEST_CASE( &girdle_test_2 ) );

  return ts;
}
