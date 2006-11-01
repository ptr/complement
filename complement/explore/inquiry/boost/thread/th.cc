#include <string>
#include <iostream>
#include <sstream>
#include <boost/thread.hpp>

using namespace std;
using namespace boost;

int main()
{
  string s( "12345678" );
  const regex re1( "^[0-9]+" );
  const regex re2( "\\d" );
  smatch m;

  cout << regex_match( s, re1 ) << endl;

  cout << regex_search( s, m, re2 ) << endl;

  cout << m.str() << endl;

//  for ( smatch::iterator i = m.begin(); i != m.end(); ++i ) {
//    cout << m.str() << endl;
//  }
  string::const_iterator b = s.begin();
  string::const_iterator e = s.end();

  while ( regex_search( b, e, m, re2 ) ) {
    cout << m.str() << endl;
    b = m[0].second;
  }
  
  return 0;
}
