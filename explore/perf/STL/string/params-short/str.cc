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
  string s( "12345678901234567890" );

  for ( int i = 0; i < 1/*0000000*/; ++i ) {
    string sx = func( s );
  }

  return 0;
}
