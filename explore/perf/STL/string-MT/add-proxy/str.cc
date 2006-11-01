// -*- C++ -*- Time-stamp: <06/10/16 21:48:50 ptr>

#include <string>
#include <pthread.h>

using namespace std;

void *f( void * )
{
  string s;
  string s1 = "1234567";
  string s2 = "12345678901234567890";
  string s3 = ".ext";
  string s4 = " /* my comment about this */";

  for ( int i = 0; i < 5000000; ++i ) {
    s = s1 + "/" + s2 + s3 + " => " + s4;
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
