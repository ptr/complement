#include "boost/date_time/gregorian/gregorian.hpp"
#include <iostream>

int main( int, char * const * )
{
  using namespace boost::gregorian;
  using namespace std;

  date d1(1970,Jan,1);
  date d2(1601,Jan,1);
  date_period dp( d2, d1 );
  date_period dp2( date(2003,Jan,1), date(2004,Jan,1) );

  cerr << "Diff is " << dp.length().days() << endl;
  cerr << "Diff is " << dp2.length().days() << endl;

  return 0;
}
