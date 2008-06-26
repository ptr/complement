#include <sstream>
#include <locale>
#include <iostream>

using namespace std;

int main()
{
  locale loc( "en_US" );

#if 0
  moneypunct<char, true> const& intl_fmp = use_facet<moneypunct<char, true> >(loc);
  cerr << "|" 
       << (int)intl_fmp.pos_format().field[0]
       << (int)intl_fmp.pos_format().field[1]
       << (int)intl_fmp.pos_format().field[2]
       << (int)intl_fmp.pos_format().field[3]
       << "|" << endl;
  ostringstream ostr;
  ostr.imbue(loc);
  ostr << showbase;
  money_put<char> const& fmp = use_facet<money_put<char> >(loc);

  ostreambuf_iterator<char> res = fmp.put(ostr, true, ostr, ' ', 123456);
  string str_res;
  str_res = ostr.str();
  cerr << str_res << endl;
#endif

  return 0;
}
