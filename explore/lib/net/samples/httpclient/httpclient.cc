// -*- C++ -*- Time-stamp: <09/07/24 00:49:46 ptr>

/*
 *
 * Copyright (c) 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <net/http.h>
#include <sockios/sockstream>

#include <iostream>
#include <iomanip>
#include <string>

using namespace std;

int main( int argc, char **argv )
{
  if ( argc != 2 ) {
    cerr << "Expected one argument: hostname" << endl;

    return 1;
  }

  http::request rq;

  rq.head().value( http::command::GET );
  rq.head().protocol( http::command::HTTP11 );
  rq.head().URL( "/" );
  rq.headers().push_back( http::header( "Host", argv[1] ) );

#if 1
  sockstream net( argv[1], 80 );

  if ( !net.good() ) {
    cerr << "can't connect to " << argv[1] << ":80" << endl;

    return 2;
  }

  (net << noskipws << http::body( true ) << rq).flush();

#if 0
  string line;

  while ( !net.fail() ) {
    getline( net, line );
    cout << line << endl;
  }
#else
  http::response rs;

  net >> noskipws >> http::body( true ) >> rs;

  cout << rs.body() << endl;
#endif

#else
  cout << noskipws << http::body( true ) << rq;
#endif

  return 0;
}
