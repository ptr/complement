// -*- C++ -*- Time-stamp: <04/01/21 18:04:57 ptr>

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "@(#)$Id: doscl.cc,v 1.2 2004/06/16 14:29:20 ptr Exp $"
#  else
#ident "@(#)$Id: doscl.cc,v 1.2 2004/06/16 14:29:20 ptr Exp $"
#  endif
#endif

#include <iostream>
#include <string>
#include <sstream>
#include <sockios/sockstream>
#include <mt/xmt.h>
#include <misc/args.h>

using namespace std;
using namespace __impl;

int command_whole_timeout_test( iostream& s, const string& hello_host, int delay )
{
  string rline;
  string err_line;

  getline( s, rline );

  cout << rline << endl;

  timespec t;
  t.tv_sec = delay;
  t.tv_nsec = 0;

  Thread::delay( &t );

  s << "EHLO " << hello_host << "\r" << endl;
  getline( s, rline );
  
  if ( s.good() ) {
    // cerr << "Connection even here seems ok" << endl;
    cout << rline << endl;
  }
  getline( s, err_line );
  if ( s.good() ) {
    return -1; // connection still ok, timeout not happens
  }

  return 0;
}

int cumulative_command_timeout_test( iostream& s, const string& hello_host, int delay )
{
  string rline;
  string err_line;

  getline( s, rline );

  cout << rline << endl;

  timespec t;
  t.tv_sec = delay;
  t.tv_nsec = 0;

  stringstream sstr;
  sstr << "EHLO " << hello_host << "\r" << endl;
  string greeting( sstr.str() );

  for ( string::iterator i = greeting.begin(); i != greeting.end(); ++i ) {
    Thread::delay( &t );
    s << *i;
    s.flush();
    (cout << *i).flush();
  }

  getline( s, rline );

  if ( s.good() ) {
    cout << rline << endl;
  }
  getline( s, err_line );
  if ( s.good() ) {
    return -1; // connection still ok, timeout not happens
  }

  // cout << rline << endl;
  return 0;
}

int main( int argc, char * const *argv )
{
  try {
    Argv arg;
    arg.copyright( "Copyright (C) K     sky Lab, 2003" );
    arg.brief( "test for SMTP GW project" );
    arg.option( "-h", false, "print this help message" );
    arg.option( "-s", string( "" ), "host with SMTP GW server" );
    arg.option( "-p", int(25), "port that listen SMTP GW server" );
    arg.option( "-hello", string( "peak.avp.ru" ), "argument of EHLO/HELO command" );
    arg.option( "-delay", int( 305 ), "delay (timeout) for test" );
    arg.option( "-test", int( 0 ), "test number (0 or 1)" );
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

    string t_host;
    int t_port;
    arg.assign( "-s", t_host );
    arg.assign( "-p", t_port );

    string hello_host;
    arg.assign( "-hello", hello_host );

    int delay;
    int test_no;
    arg.assign( "-delay", delay );
    arg.assign( "-test", test_no );

    string rline;

    sockstream s( t_host.data(), t_port );
    if ( test_no == 0 ) {
      return command_whole_timeout_test( s, hello_host, delay );
    } else if ( test_no == 1 ) {
      return cumulative_command_timeout_test( s, hello_host, delay );
    }

    return -1;
  }
  catch( int ret ) {
    return ret;
  }
  
  return 0;
}
