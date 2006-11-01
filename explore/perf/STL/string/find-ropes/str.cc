// -*- C++ -*- Time-stamp: <03/04/05 22:03:42 ptr>

#include <rope>

using namespace std;

int main( int, char * const * )
{
  rope<char> s( "qyweyuewunfkHBUKGYUGL,wehbYGUW^\
(@T@H!BALWD:h^&@#*@(#:JKHWJ:CND" );

  for ( int i = 0; i < 10000000; ++i ) {
    s.find( "unfkHBUKGY" );
    s.find( "W^(@T@H!B" );
    s.find( "J:CND" );
  }

  return 0;
}
