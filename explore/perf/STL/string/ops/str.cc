// -*- C++ -*- Time-stamp: <03/04/05 22:13:14 ptr>

#include <string>

using namespace std;

int main( int, char * const * )
{
  string s( "qyweyuewunfkHBUKGYUGL,wehbYGUW^\
(@T@H!BALWD:h^&@#*@(#:JKHWJ:CND" );
  string::size_type p;
  string ss1( "unfkHBUKGY" );
  string ss2( "123456" );
  string sx;

  for ( int i = 0; i < 10000000; ++i ) {
    sx = s;
    p = sx.find( ss1 );
    sx.replace( p, ss1.size(), ss2 );
    sx += s;
  }

  return 0;
}
