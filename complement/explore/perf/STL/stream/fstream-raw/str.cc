// -*- C++ -*- Time-stamp: <05/04/27 18:47:41 ptr>

#include <fstream>

using namespace std;

int main( int, char * const * )
{
  ofstream s( "test" );

  for ( int i = 0; i < 10000000; ++i ) {
    s.write( (const char *)&i, sizeof(i) );
  }

  return 0;
}
