#include <string>
#include <iostream>
#include <sstream>
#include <boost/regex.hpp>

using namespace std;
using namespace boost;

int main()
{
  string s( "12345678" );
  const regex re( "^[0-9]+" );
  // smatch m;

  cout << regex_match( s, re ) << endl;
  
  return 0;
}
