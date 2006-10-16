// -*- C++ -*- Time-stamp: <03/04/04 23:07:39 ptr>

#include <string>

using namespace std;

int main( int, char * const * )
{
  string s;
  string s1 = "1234567";
  string s2 = "12345678901234567890";
  string s3 = ".ext";
  string s4 = " /* my comment about this */";

  for ( int i = 0; i < 100000000; ++i ) {
    s = s1 + "/" + s2 + s3 + " => " + s4;
  }

  return 0;
}
