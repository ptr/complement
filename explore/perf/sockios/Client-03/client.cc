// -*- C++ -*- Time-stamp: <02/12/01 10:45:57 ptr>

/*
 *
 * Copyright (c) 2002
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License Version 1.0
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 *
 */

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id$"
#  else
#ident "@(#)$Id$"
#  endif
#endif

#include <mt/xmt.h>
#include <sockios/sockstream>
#include <sockios/sockmgr.h>
#include <misc/args.h>

#include <iostream>
#include <string>
#include <vector>

using namespace __impl;
using namespace std;

int main( int argc, char * const *argv )
{
  int port;
  string host;
  int ni;
  int bs;

  try {
    Argv arg;
    arg.copyright( "Copyright (C) Petr Ovtchenkov, 2002" );
    arg.brief( "Performance measure client" );
    arg.option( "-h", false, "print this help message" );
    arg.option( "-p", int( 1990 ), "Connect to tcp port (1990)" );
    arg.option( "-host", std::string( "localhost" ), "Connect to host (localhost)" );
    arg.option( "-n", int( 10000 ), "number of iterations (10000)" );
    arg.option( "-b", int( 1024 ), "block size (1024)" );
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
    arg.assign( "-b", bs );
  }
  catch ( std::runtime_error& err ) {
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

  std::sockstream sock( host.data(), port );
  // string b( 10240, 'x' );
  char *b = new char [bs];

  for ( int i = 0; i < ni; ++i ) {
    if ( sock.good() ) {
      // sock << b; // << endl;
      sock.write( b, bs );
    } else {
      cerr << "Something wrong (" << i << ")" << endl;
      break;
    }
  }
  delete b;

  return 0;
}
