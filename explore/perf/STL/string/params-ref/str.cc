// -*- C++ -*- Time-stamp: <04/07/14 23:40:29 ptr>

#include <string>
#include <pthread.h>

using namespace std;

string func( const string& par )
{
  string tmp( par );

  return tmp;
}

int main( int, char * const * )
{
  string s( "qyweyuewunfkHBUKGYUGL,wehbYGUW^\
(@T@H!BALWD:h^&@#*@(#:JKHWJ:CND" );

  for ( int i = 0; i < 10000000; ++i ) {
    string sx = func( s );
  }

  return 0;
}
