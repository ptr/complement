// -*- C++ -*- Time-stamp: <05/04/27 18:16:09 ptr>

#include <sstream>

using namespace std;

int main( int, char * const * )
{
  stringstream s;

  for ( int i = 0; i < 10000000; ++i ) {
    s.write( (const char *)&i, sizeof(i) );
  }

  return 0;
}
