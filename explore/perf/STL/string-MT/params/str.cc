// -*- C++ -*- Time-stamp: <03/07/23 23:12:18 ptr>

#include <string>
#include <pthread.h>

using namespace std;

string func( string par )
{
  string tmp( par );

  return tmp;
}

void *f( void * )
{
  string s( "qyweyuewunfkHBUKGYUGL,wehbYGUW^\
(@T@H!BALWD:h^&@#*@(#:JKHWJ:CND" );

  for ( int i = 0; i < 20000000; ++i ) {
    string sx = func( s );
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
