// -*- C++ -*- Time-stamp: <03/04/04 23:07:39 ptr>

#include <string>

using namespace std;

int main( int, char * const * )
{
  string s;

  for ( int i = 0; i < 100000000; ++i ) {
    s += " ";
  }

  return 0;
}
