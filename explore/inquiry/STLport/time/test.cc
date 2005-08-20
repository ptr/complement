// -*- C++ -*- Time-stamp: <02/09/25 12:11:17 ptr>

#include <iostream>
#include <sstream>
#include <locale>
#include <ctime>

using namespace std;

int main( int argc, char * const *argv )
{
  typedef istreambuf_iterator<char, char_traits<char> > itype;
  locale loc("en_US");
  time_get<char,itype> const & t = use_facet<time_get<char,itype> >( loc );
  istringstream s( "05-06-2005 21:34:41" );

  cout << t.date_order() << endl;

  tm ts;
  ios_base::iostate state;
  t.get_date( itype(s), itype(), s, state, &ts );
  
  cout << ts.tm_mday << "-"
       << ts.tm_mon << "-"
       << ts.tm_year << " "
       << ts.tm_hour << ":"
       << ts.tm_min << ":"
       << ts.tm_sec << endl;

  return 0;
}
