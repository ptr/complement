// -*- C++ -*- Time-stamp: <03/07/23 23:03:00 ptr>

#include <string>

using namespace std;

string func( string par )
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
