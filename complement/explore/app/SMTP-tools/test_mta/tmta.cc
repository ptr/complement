#include "ConnectionProcessor.h"
#include <sockios/sockmgr.h>
#include <misc/args.h>

#include <iostream>
#include <iomanip>

using namespace std;

extern bool trace_flag;
extern string tecol_host;
extern int tecol_port;

int main( int argc, char * const *argv )
{
  int port;

  try {
    Argv arg;
    arg.copyright( "Copyright (C) K     sky Lab, 2003, 2004" );
    arg.brief( "Ttrivial mail transfer agent" );
    arg.option( "-h", false, "print this help message" );
    arg.option( "-p", int( 10026 ), "listen port, default 10026" );
    arg.option( "-t", false, "trace events" );
    arg.option( "-tecol", string( "localhost" ), "host running tecol server, localhost default" );
    arg.option( "-tecolport", int(2049), "tecol port, 2049 default" );
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
    arg.assign( "-tecol", tecol_host );
    arg.assign( "-tecolport", tecol_port );
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
