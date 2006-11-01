// -*- C++ -*- Time-stamp: <02/09/10 14:44:21 ptr>

/*
 *
 * Copyright (c) 2002
 * Petr Ovtchenkov
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

#include <sockios/sockstream>
#include <string>
#include <misc/args.h>
#include <iostream>

using namespace std;

int main( int argc, char * const *argv )
{
  int port;
  int ni;
  std::string host;

  try {
    Argv arg;
    arg.copyright( "Copyright (C) Petr Ovtchenkov, 2002" );
    arg.brief( "Performance measure client" );
    arg.option( "-h", false, "print this help message" );
    arg.option( "-p", int( 1990 ), "Connect to tcp port (1990)" );
    arg.option( "-host", std::string( "localhost" ), "Connect to host (localhost)" );
    arg.option( "-n", int( 10000), "number of iterations (10000)" );
    try {
      arg.parse( argc, argv );
    }
    catch ( std::invalid_argument& err ) {
      cerr << err.what() << endl;
      arg.print_help( cerr );
      throw 1;
    }
    bool turn;
    if ( arg.assign( "-h", turn ) ) {
      arg.print_help( cerr );
      throw 0;
    }
    arg.assign( "-p", port );
    arg.assign( "-host",  host);
    arg.assign( "-n", ni );
  }
  catch ( runtime_error& err ) {
    cerr << err.what() << endl;
    return -1;
  }
  catch ( std::exception& err ) {
    cerr << err.what() << endl;
    return -1;
  }
  catch ( int r ) {
    return r;
  }


  std::sockstream sock;
  
  for ( int i = 0; i < ni; ++i ) {
    sock.open( host.data(), port );
    sock.close();
  }

  return 0;
}
