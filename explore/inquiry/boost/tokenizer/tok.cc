#include <string>
#include <iostream>
#include <sstream>
#include <boost/tokenizer.hpp>

using namespace std;
using namespace boost;

int main()
{
  string s( "Test, here , test,test  only   !!!\nOk?" );

  tokenizer<> tok( s );
  for ( tokenizer<>::iterator i = tok.begin(); i != tok.end(); ++i ) {
    cout << *i << endl;
  }

  typedef tokenizer<char_separator<char> > tok_sep_class;
  char_separator<char> sep( ",! ?\n" );
  tok_sep_class toks( s, sep );

  for ( tok_sep_class::iterator i = toks.begin(); i != toks.end(); ++i ) {
    cout << *i << endl;
  }  

  return 0;
}
