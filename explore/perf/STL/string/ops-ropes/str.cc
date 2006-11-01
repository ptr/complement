// -*- C++ -*- Time-stamp: <03/04/05 22:13:14 ptr>

#include <rope>

using namespace std;

int main( int, char * const * )
{
  rope<char> s( "qyweyuewunfkHBUKGYUGL,wehbYGUW^\
(@T@H!BALWD:h^&@#*@(#:JKHWJ:CND" );
  rope<char>::size_type p;
  rope<char> ss1( "unfkHBUKGY" );
  rope<char> ss2( "123456" );
  rope<char> sx;

  for ( int i = 0; i < 10000000; ++i ) {
    sx = s;
    p = sx.find( ss1 );
    sx.replace( p, ss1.size(), ss2 );
    sx += s;
  }

  return 0;
}
