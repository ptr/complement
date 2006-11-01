// -*- C++ -*- Time-stamp: <03/06/26 00:27:02 ptr>

#include <string>
#include <pthread.h>

using namespace std;

void *f( void * )
{
  string s;

  for ( int i = 0; i < 100000000; ++i ) {
    s += " ";
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
