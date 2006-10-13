// -*- C++ -*- Time-stamp: <04/07/14 23:39:44 ptr>

#include <rope>

using namespace std;

rope<char> func( rope<char> par )
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
