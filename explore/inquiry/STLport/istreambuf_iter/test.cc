#include <iostream>
// #include <fstream>
#include <string>
#include <vector>
#include <iterator>
#include <sstream>

using namespace std;

int main()
{
  // ifstream f;
  stringstream s( "1234567890" );
  stringstream g( "1234567890xx" );
  char buf[] = "12345678901234";
  string line;
  vector<char> v;

  // f.open( "test.txt", fstream::in );
  // v.assign( istreambuf_iterator<char>(f), istreambuf_iterator<char>() );

  v.assign( istreambuf_iterator<char>(s), istreambuf_iterator<char>() );
  cerr << v.size() << endl;
  
  for ( vector<char>::const_iterator i = v.begin(); i != v.end(); ++i ) {
    cerr << *i;
  }

  cerr << endl;  

  vector<char>::const_iterator j = v.begin() + 5;

  v.assign( istreambuf_iterator<char>(g), istreambuf_iterator<char>() );

  j += 1;

  cerr << *j << endl;

  v.assign( buf, buf + 14);

  for ( vector<char>::const_iterator i = v.begin(); i != v.end(); ++i ) {
    cerr << *i;
  }

  cerr << endl;

  j += 1;

  cerr << *j << endl;

  return 0;
}
