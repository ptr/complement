// -*- C++ -*- Time-stamp: <03/04/13 19:24:46 ptr>

#include <misc/args.h>

#include <rope>
#include <string>
#include <iostream>

using namespace std;

template <class T>
class test
{
  public:
    test( int b, int i );
};

template <class T>
test<T>::test( int b, int n )
{
  T s( b, 'a' );
  T v;
  for ( int i = 0; i < n; ++i ) {
    v = s;
    v.insert( 0, "qwerty");
    v.insert( b / 2, "zxcvb" );
    v.append( "rtyui" );
    v.replace( 0, 20U, "ghfjhfjf" );
    v.replace( b / 2, 20U, "abcdefg" );
  }
}

int bs;
int n;
bool use_str = true;

int main( int argc, char * const *argv )
{
  try {
    Argv arg;
    arg.copyright( "Copyright (C) Petr Ovchenkov, 2003" );
    arg.brief( "Comparison of ropes and strings" );
    arg.option( "-h", false, "print this help message" );
    arg.option( "-r", true, "use ropes" );
    arg.option( "-s", true,  "use strings" );
    arg.option( "-b", 1, "block size" );
    arg.option( "-i", 1, "number of iterations" );
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

    if ( arg.is( "-r" ) && arg.is( "-s" ) ) {
      cerr << "Either -r or -s allowed" << endl;
    }

    arg.assign( "-s", use_str );

    if ( arg.is( "-r" ) ) {
      use_str = false;
    }
    arg.assign( "-b", bs );
    arg.assign( "-i", n );
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

  if ( !use_str ) {
    test<rope<char> > t( bs, n );
  } else {
    test<string> t( bs, n );
  }

  return 0;
}
