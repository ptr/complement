// -*- C++ -*- Time-stamp: <03/07/24 08:47:54 ptr>

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

  for ( int i = 0; i < 20000000; ++i ) {
    string sx = func( s );
  }

  return 0;
}
