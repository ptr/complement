// -*- C++ -*- Time-stamp: <03/06/26 09:07:24 ptr>

#include <rope>
#include <pthread.h>

using namespace std;

void *f( void * )
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
