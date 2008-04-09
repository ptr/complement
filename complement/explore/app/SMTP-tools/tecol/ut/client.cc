// -*- C++ -*- Time-stamp: <03/09/17 10:57:50 ptr>

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id: client.cc,v 1.1 2003/10/06 08:01:06 ptr Exp $"
#  else
#ident "@(#)$Id: client.cc,v 1.1 2003/10/06 08:01:06 ptr Exp $"
#  endif
#endif

#include <boost/test/test_tools.hpp>

#include <string>
#include <sockios/sockstream>
#include <iostream>
#include <iomanip>
#include <mt/xmt.h>

#include "client.h"
#include "message.h"

using namespace std;
using namespace __impl;

void Client::client1()
{
  BOOST_MESSAGE( "Client start" );
  std::sockstream sock( "localhost", 2049 );
  string srv_line;

  sock << ::msg << endl;

  BOOST_CHECK( sock.good() );

  // sock.clear();
  getline( sock, srv_line );

  BOOST_CHECK( sock.good() );

  BOOST_CHECK_EQUAL( srv_line, ::msg_rsp );

  BOOST_MESSAGE( "Client close connection (client's end of life)" );
  // sock.close(); // no needs, that will done in sock destructor
}
