// -*- C++ -*- Time-stamp: <05/04/27 18:47:41 ptr>

#include <fstream>

using namespace std;

int main( int, char * const * )
{
  ofstream s( "test" );

  for ( int i = 0; i < 1000000; ++i ) {
    s << i << " " << (static_cast<double>(i) + 0.1415926) << ends;
  }

  return 0;
}
