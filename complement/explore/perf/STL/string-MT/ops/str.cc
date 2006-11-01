// -*- C++ -*- Time-stamp: <03/06/26 09:07:22 ptr>

#include <string>
#include <pthread.h>

using namespace std;

void *f( void * )
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

int main( int, char * const * )
{
  const int nth = 2;
  pthread_t t[nth];

  for ( int i = 0; i < nth; ++i ) {
    pthread_create( &t[i], 0, f, 0 );
  }

  for ( int i = 0; i < nth; ++i ) {
    pthread_join( t[i], 0 );
  }

  return 0;
}
