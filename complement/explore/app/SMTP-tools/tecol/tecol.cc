// -*- C++ -*- Time-stamp: <04/05/25 14:35:15 ptr>

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id: tecol.cc,v 1.2 2004/06/16 14:26:39 ptr Exp $"
#  else
#ident "@(#)$Id: tecol.cc,v 1.2 2004/06/16 14:26:39 ptr Exp $"
#  endif
#endif


#include "ConnectionProcessor.h"
#include <sockios/sockmgr.h>
#include <misc/args.h>

#include <iostream>
#include <iomanip>

using namespace std;

extern bool trace_flag;

int main( int argc, char * const *argv )
{
  int port;

  try {
    Argv arg;
    arg.copyright( "Copyright (C) K     sky Lab, 2003, 2004" );
    arg.brief( "Tests notification collection server" );
    arg.option( "-h", false, "print this help message" );
    arg.option( "-p", int( 2049 ), "listen port, default 2049" );
    arg.option( "-t", false, "trace events" );
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
      arg.print_help( cout );
      throw 0;
    }
    arg.assign( "-p", port );
    arg.assign( "-t", trace_flag );
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

  sockmgr_stream_MP<ConnectionProcessor> srv( port );

  srv.wait();
  
  return 0;
}
