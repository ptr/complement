// -*- C++ -*- Time-stamp: <03/07/24 08:47:57 ptr>

#include <rope>
#include <pthread.h>

using namespace std;

rope<char> func( const rope<char>& par )
{
  rope<char> tmp( par );

  return tmp;
}

void *f( void * )
{
  rope<char> s( "qyweyuewunfkHBUKGYUGL,wehbYGUW^\
(@T@H!BALWD:h^&@#*@(#:JKHWJ:CND" );

  for ( int i = 0; i < 10000000; ++i ) {
   rope<char> sx = func( s );
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
