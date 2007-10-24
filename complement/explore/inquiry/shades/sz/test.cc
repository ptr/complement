#include <iostream>
// #include <string>

char *arr[][2] = {
  { "1", "1.2" },
  { "2", "2.2" },
  { "2", "2.2" }
};

using namespace std;

int main()
{
  cerr << sizeof(arr) << " " << sizeof(arr[0]) << " " << sizeof(arr)/sizeof(arr[0]) << endl;

#if 0
  string s( "01234567Tabc" );

  string::size_type p = s.find( 'T', 6 );
  if ( p != string::npos ) {
    cerr << s.substr( 0, p ) << endl;
  }

  s.erase( p );
  cerr << s << endl;
#endif
  return 0;
}

