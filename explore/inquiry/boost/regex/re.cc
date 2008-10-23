#include <string>
#include <iostream>
#include <sstream>
#include <boost/regex.hpp>

using namespace std;
using namespace boost;

int main()
{
#if 0
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
#endif
#if 1
  string s( "L=DGhFBHBqSWNZWQN/XHlmc2FwfVN/C1VsHTg0GlsAKBg1FD9VGgRYBh8COC0xDiMjTlxJOSk/cQ4ZIAN/Lm5+CQ==.1193829498.1711.233703.f3ced884c7fb604c9a53e562ff921344; path=/; domain=.yandex.ru; expires=Tuesday, 29-Jan-08 11:18:18 GMT" );
  // boost::regex cookie_re( "^(?:(\\w+)=([^;]*?)(?:;[[:space:]]+))*?(?:(\\w+)=([^;]*?))$" );
  boost::regex cookie_re( "(?:(\\w+)=([^;]*)(?:;\\s+)?)*" );
  boost::smatch ma;
  if ( regex_search( s, ma, cookie_re, boost::match_extra ) ) {
    for ( int i = 1; i < ma.size(); ++i ) {
      cout << ma[i] << endl;
#ifdef BOOST_REGEX_MATCH_EXTRA
      cout << "@ ";
      for ( int j = 0; j < ma.captures(i).size(); ++j ) {
        cout << ma.captures(i)[j] << "%";
      }
      cout << endl;
#endif
    }
  }
#endif
  
  return 0;
}
