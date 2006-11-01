// -*- C++ -*- Time-stamp: <04/07/15 23:56:40 ptr>

#include <string>

using namespace std;

string func( string par )
{
  string tmp( par );

  return tmp;
}

int main( int, char * const * )
{
  string s( "1234567890" );

  for ( int i = 0; i < 10000000; ++i ) {
    string sx = func( s );
  }

  return 0;
}
