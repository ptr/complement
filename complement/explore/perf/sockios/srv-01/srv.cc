// -*- C++ -*- Time-stamp: <02/09/10 14:45:07 ptr>

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

#include <mt/xmt.h>
#include <sockios/sockstream>
#include <sockios/sockmgr.h>

#include <iostream>
#include <fstream>
#include <string>
#include <misc/args.h>

#include <sys/types.h>
#include <unistd.h>

#include "ConnectionProcessor.h"

// using namespace __impl;
using namespace std;


int main( int argc, char * const *argv )
{
  int port;
  std::string host;
  std::string fname;

  try {
    Argv arg;
    arg.copyright( "Copyright (C) Petr Ovtchenkov, 2002" );
    arg.brief( "Performance measure server" );
    arg.option( "-h", false, "print this help message" );
    arg.option( "-p", int( 1990 ), "Listen tcp port (1990)" );
    arg.option( "-pid", string( "srv.pid" ), "File to store pid (srv.pid)" );
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
    arg.assign( "-pid", fname );
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

  pid_t pid = getpid();

  sockmgr_stream_MP<ConnectionProcessor> srv( port );

  ofstream pf( fname.data() );
  pf << pid << endl;
  pf.close();

  srv.wait();

  return 0;
}
