// -*- C++ -*- Time-stamp: <04/02/18 19:42:59 ptr>

#include <iostream>
#include <vector>

using namespace std;

int main( int argc, char * const *argv )
{
  typedef vector<char *> container_type;

  container_type cnt( 3 );

  cerr << cnt.size() << endl;

  cnt[0] = "Hello";

  int j = 0;
  for ( container_type::reverse_iterator i = cnt.rbegin(); i != cnt.rend(); ++i ) {
    cerr << j << ": " << (*i != 0 ? *i : "" ) << endl;
    if ( *i != 0 ) {
      cerr << "  last_value " << (cnt.rend() - i) << endl;
    }
    ++j;
  }

  return 0;
}
