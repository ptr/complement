// -*- C++ -*- Time-stamp: <03/07/24 08:47:54 ptr>

#include <rope>

using namespace std;

rope<char> func( const rope<char>& par )
{
  rope<char> tmp( par );

  return tmp;
}

int main( int, char * const * )
{
  rope<char> s( "qyweyuewunfkHBUKGYUGL,wehbYGUW^\
(@T@H!BALWD:h^&@#*@(#:JKHWJ:CND" );

  for ( int i = 0; i < 10000000; ++i ) {
    rope<char> sx = func( s );
  }

  return 0;
}
