// -*- C++ -*- Time-stamp: <98/11/12 20:45:43 ptr>
#ifndef __CyrMoney_h
#define __CyrMoney_h

#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#include <string>

using std::string;

class cyr_money_converter
{
  public:
    enum encoding {
      ISO8859_5,
      CP1251,
      CP866,
      KOI8,
      LAST_ENCODING
    };
 
    static string conv( int n, encoding = ISO8859_5 );
    static string conv( const string& n, encoding = ISO8859_5 );

  private:
    static string x100( int x, encoding enc = ISO8859_5, bool alt = false );
    static string x1000( int u, string s0, string s1, string s2,
			 encoding enc = ISO8859_5, bool alt = false );

};

#endif // __CyrMoney_h
