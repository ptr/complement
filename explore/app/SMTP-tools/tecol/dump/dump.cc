// -*- C++ -*- Time-stamp: <04/06/17 10:57:48 ptr>

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id: dump.cc,v 1.1 2005/05/10 15:13:23 ptr Exp $"
#  else
#ident "@(#)$Id: dump.cc,v 1.1 2005/05/10 15:13:23 ptr Exp $"
#  endif
#endif

#include <sockios/sockmgr.h>
#include <misc/args.h>

#include <iostream>
#include <iomanip>

using namespace std;

extern bool trace_flag;

int main( int argc, char * const *argv )
{
  int port;
  string host;

  try {
    Argv arg;
    arg.copyright( "Copyright (C) K     sky Lab, 2004" );
    arg.brief( "Dump times from tecol server" );
    arg.option( "-h", false, "print this help message" );
    arg.option( "-p", int( 2049 ), "tecol server port, default 2049" );
    arg.option( "-s", string("localhost"), "tecol server host, default localhost" );
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
    arg.assign( "-s", host );
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

  sockstream s( host.c_str(), port );

  if ( !s.good() ) {
    return -1;
  }

  string trash;
  s << "action=lstf" << endl;
  getline( s, trash );

  double abs_time;
  double proc_time;
  cout.setf(ios_base::fixed);
  while ( s.good() ) {
    s >> abs_time >> trash >> proc_time;
    if ( s.good() ) {
      cout << abs_time << ' ' << proc_time << '\n';
    }
  }
  
  return 0;
}
