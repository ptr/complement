// -*- C++ -*- Time-stamp: <03/09/16 12:02:29 ptr>

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id: unit_test.cc,v 1.1 2003/10/06 08:01:06 ptr Exp $"
#  else
#ident "@(#)$Id: unit_test.cc,v 1.1 2003/10/06 08:01:06 ptr Exp $"
#  endif
#endif

#include <boost/test/unit_test.hpp>

using namespace boost::unit_test_framework;

#include <iostream>
#include <mt/xmt.h>

#include <sockios/sockstream>
#include <sockios/sockmgr.h>

using namespace std;

#include "../ConnectionProcessor.h"
#include "client.h"

void test_client_server()
{
  sockmgr_stream_MP<ConnectionProcessor> srv( 2049 );

  Client::client1();
  
  srv.close();
  srv.wait();
}

test_suite *init_unit_test_suite( int argc, char **argv )
{
  test_suite *ts = BOOST_TEST_SUITE( "tecol test" );

  ts->add( BOOST_TEST_CASE( &test_client_server ) );

  return ts;
}
