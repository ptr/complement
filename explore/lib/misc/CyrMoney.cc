// -*- C++ -*- Time-stamp: <99/03/13 23:03:20 ptr>
#ident "%Z% $Date$ $Revision$ $RCSfile$ %Q%"

#include <CyrMoney.h>
#include <sstream>

using namespace std;

string theUnits_ISO8859_5[] =
{ "", "ÞÔØÝ", "ÔÒÐ", "âàØ", "çÕâëàÕ", "ßïâì", "èÕáâì", "áÕÜì", "ÒÞáÕÜì",
  "ÔÕÒïâì", "ÔÕáïâì", "ÞÔØÝÝÐÔæÐâì", "ÔÒÕÝÐÔæÐâì", "âàØÝÐÔæÐâì",
  "çÕâëàÝÐÔæÐâì", "ßïâÝÐÔæÐâì", "èÕáâÝÐÔæÐâì", "áÕÜÝÐÔæÐâì", "ÒÞáÕÜÝÐÔæÐâì",
  "ÔÕÒïâÝÐÔæÐâì"
};

string theUnits_CP1251[] =
{ "", "îäèí", "äâà", "òðè", "÷åòûðå", "ïÿòü", "øåñòü", "ñåìü", "âîñåìü",
  "äåâÿòü", "äåñÿòü", "îäèííàäöàòü", "äâåíàäöàòü", "òðèíàäöàòü",
  "÷åòûðíàäöàòü", "ïÿòíàäöàòü", "øåñòíàäöàòü", "ñåìíàäöàòü", "âîñåìíàäöàòü",
  "äåâÿòíàäöàòü"
};

string *theUnits[] = { theUnits_ISO8859_5, theUnits_CP1251 };

string theUnitsAlt_ISO8859_5[] =
{
  "", "ÞÔÝÐ", "ÔÒÕ", "âàØ", "çÕâëàÕ", "ßïâì", "èÕáâì", "áÕÜì", "ÒÞáÕÜì",
  "ÔÕÒïâì", "ÔÕáïâì", "ÞÔØÝÝÐÔæÐâì", "ÔÒÕÝÐÔæÐâì", "âàØÝÐÔæÐâì",
  "çÕâëàÝÐÔæÐâì", "ßïâÝÐÔæÐâì", "èÕáâÝÐÔæÐâì", "áÕÜÝÐÔæÐâì", "ÒÞáÕÜÝÐÔæÐâì",
  "ÔÕÒïâÝÐÔæÐâì"
};

string theUnitsAlt_CP1251[] =
{
  "", "îäíà", "äâå", "òðè", "÷åòûðå", "ïÿòü", "øåñòü", "ñåìü", "âîñåìü",
  "äåâÿòü", "äåñÿòü", "îäèííàäöàòü", "äâåíàäöàòü", "òðèíàäöàòü",
  "÷åòûðíàäöàòü", "ïÿòíàäöàòü", "øåñòíàäöàòü", "ñåìíàäöàòü", "âîñåìíàäöàòü",
  "äåâÿòíàäöàòü"
};

string *theUnitsAlt[] = { theUnitsAlt_ISO8859_5, theUnitsAlt_CP1251 };

string theDecs_ISO8859_5[] = {
 "", "", "ÔÒÐÔæÐâì", "âàØÔæÐâì", "áÞàÞÚ", "ßïâìÔÕáïâ", "èÕáâìÔÕáïâ",
 "áÕÜìÔÕáïâ", "ÒÞáÕÜìÔÕáïâ", "ÔÕÒïÝÞáâÞ"
};

string theDecs_CP1251[] = {
 "", "", "äâàäöàòü", "òðèäöàòü", "ñîðîê", "ïÿòüäåñÿò", "øåñòüäåñÿò",
 "ñåìüäåñÿò", "âîñåìüäåñÿò", "äåâÿíîñòî"
};

string *theDecs[] = { theDecs_ISO8859_5, theDecs_CP1251 };

string theHund_ISO8859_5[] = {
 "", "áâÞ", "ÔÒÕáâØ", "âàØáâÐ", "çÕâëàÕáâÐ", "ßïâìáÞâ",
 "èÕáâìáÞâ", "áÕÜìáÞâ", "ÒÞáÕÜìáÞâ", "ÔÕÒïâìáÞâ"
};

string theHund_CP1251[] = {
 "", "ñòî", "äâåñòè", "òðèñòà", "÷åòûðåñòà", "ïÿòüñîò",
 "øåñòüñîò", "ñåìüñîò", "âîñåìüñîò", "äåâÿòüñîò"
};

string *theHund[] = { theHund_ISO8859_5, theHund_CP1251 };

string bills_ISO8859_5[] = { " ÜØÛÛØÐàÔ", " ÜØÛÛØÐàÔÐ", " ÜØÛÛØÐàÔÞÒ" };
string bills_CP1251[] = {  " ìèëëèàðä", " ìèëëèàðäà", " ìèëëèàðäîâ" };

string *bills[] = { bills_ISO8859_5, bills_CP1251 };

string mills_ISO8859_5[] = { " ÜØÛÛØÞÝ", " ÜØÛÛØÞÝÐ", " ÜØÛÛØÞÝÞÒ" };
string mills_CP1251[] = { " ìèëëèîí", " ìèëëèîíà", " ìèëëèîíîâ" };

string *mills[] = { mills_ISO8859_5, mills_CP1251 };

string thous_ISO8859_5[] = { " âëáïçÐ", " âëáïçØ", " âëáïç" };
string thous_CP1251[] = { " òûñÿ÷à", " òûñÿ÷è", " òûñÿ÷" };

string *thous[] = { thous_ISO8859_5, thous_CP1251 };

string rubles_ISO8859_5[] = { " àãÑÛì", " àãÑÛï", " àãÑÛÕÙ" };
string rubles_CP1251[] = { " ðóáëü", " ðóáëÿ", " ðóáëåé" };

string *rubles[] = { rubles_ISO8859_5, rubles_CP1251 };

string null_rubles_ISO8859_5[] = { " àãÑÛÕÙ", "ÝÞÛì àãÑÛÕÙ" };
string null_rubles_CP1251[] = { " ðóáëåé", "íîëü ðóáëåé" };

string *null_rubles[] = { null_rubles_ISO8859_5, null_rubles_CP1251 };

string cents_ISO8859_5[] = { " ÚÞßÕÙÚÐ", " ÚÞßÕÙÚØ", " ÚÞßÕÕÚ" };
string cents_CP1251[] = { " êîïåéêà", " êîïåéêè", " êîïååê" };

string *cents[] = { cents_ISO8859_5, cents_CP1251 };

string months_CP1251[] = { "ÿíâàðÿ", "ôåâðàëÿ", "ìàðòà", "àïðåëÿ",
"ìàÿ", "èþíÿ", "èþëÿ", "àâãóñòà", "ñåíòÿáðÿ", "îêòÿáðÿ", "íîÿáðÿ", "äåêàáðÿ" };

string months_ISO8859_5[] = { "ïÝÒÐàï", "äÕÒàÐÛï", "ÜÐàâÐ", "ÐßàÕÛï",
"ÜÐï", "ØîÝï", "ØîÛï", "ÐÒÓãáâÐ", "áÕÝâïÑàï", "ÞÚâïÑàï", "ÝÞïÑàï", "ÔÕÚÐÑàï" };

string *monthes[] = { months_ISO8859_5, months_CP1251 };

string cyr_money_converter::x1000( int u, string s0, string s1, string s2,
				   encoding enc, bool alt )
{
  string res;

  if ( u > 0 ) {
    res += x100( u, enc, alt );
    if ( u % 10 == 1 && (u % 100) / 10 != 1 ) {
      res += s0;
    } else if ( u % 10 < 5 && (u % 100) / 10 != 1 && u % 10 > 0 ) {
      res += s1;
    } else {
      res += s2;
    }
  }

  return res;
}

string cyr_money_converter::x100( int x, encoding enc, bool alt )
{
  int units = x % 10; x /= 10;
  int decs  = x % 10; x /= 10;
  int hunds = x % 10; x /= 10;
  if ( decs == 1 ) {
    units += 10;
    decs = 0;
  }

  string res;

  if ( hunds > 0 ) {
    res += ' ';
    res += theHund[enc][hunds];
  }
  if ( decs > 0 ) {
    res += ' ';
    res += theDecs[enc][decs];
  }
  if ( units > 0 ) {
    res += ' ';
    res += alt ? theUnitsAlt[enc][units] : theUnits[enc][units];
  }


  return res;
}

string cyr_money_converter::conv( int n, cyr_money_converter::encoding enc )
{
  int units     = n % 1000; n /= 1000;
  int thousands = n % 1000; n /= 1000;
  int millions  = n % 1000; n /= 1000;
  int billions  = n;

  string res;

  res += x1000( billions, bills[enc][0], bills[enc][1], bills[enc][2], enc );
  res += x1000( millions, mills[enc][0], mills[enc][1], mills[enc][2], enc );
  res += x1000( thousands, thous[enc][0], thous[enc][1], thous[enc][2], enc, true );
  res += x1000( units, rubles[enc][0], rubles[enc][1], rubles[enc][2], enc );
  if ( units == 0 ) {
    if ( billions != 0 || millions != 0 || thousands != 0 ) {
      res += null_rubles[enc][0];
    } else {
      res += null_rubles[enc][1];
    }
  }

  return res;
}

string cyr_money_converter::conv( const string& n, cyr_money_converter::encoding enc )
{
  string nn( n );
  stringstream ss;
  string::size_type point = nn.find( '.' );
  string s_cents;
  if ( point != string::npos ) {
    s_cents = nn.substr( point + 1 );
    nn.erase( point );
    ss.str( s_cents );
    int u;
    ss >> u;
    if ( u % 10 == 1 && (u % 100) / 10 != 1 ) {
      s_cents += cents[enc][0];
    } else if ( u % 10 < 5 && (u % 100) / 10 != 1 && u % 10 > 0 ) {
      s_cents += cents[enc][1];
    } else {
      s_cents += cents[enc][2];
    }
  } else {
    s_cents = "00" + cents[enc][2];
  }
  string::size_type sz = nn.length();

  int units = 0;
  int thousands = 0;
  int millions = 0;
  int billions = 0;

  if ( sz > 3 ) {
    sz -= 3;
    ss.str( nn.substr( sz ) );
    ss >> units;
    nn.erase( sz );
    if ( sz > 3 ) {
      sz -= 3;
      ss.str( nn.substr( sz ) );
      ss >> thousands;
      nn.erase( sz );
      if ( sz > 3 ) {
	sz -= 3;
	ss.str( nn.substr( sz ) );
	ss >> millions;
	nn.erase( sz );
	ss.str( nn );
	ss >> billions;
      } else {
	ss.str( nn );
	ss >> millions;
      }
    } else {
      ss.str( nn );
      ss >> thousands;
    }
  } else {
    ss.str( nn );
    ss >> units;
  }

  string res;

  res += x1000( billions, bills[enc][0], bills[enc][1], bills[enc][2], enc );
  res += x1000( millions, mills[enc][0], mills[enc][1], mills[enc][2], enc );
  res += x1000( thousands, thous[enc][0], thous[enc][1], thous[enc][2], enc, true );
  res += x1000( units, rubles[enc][0], rubles[enc][1], rubles[enc][2], enc );
  if ( units == 0 ) {
    if ( billions != 0 || millions != 0 || thousands != 0 ) {
      res += null_rubles[enc][0];
    } else {
      res += null_rubles[enc][1];
    }
  }
  res += " " + s_cents;

  string::size_type fp = res.find_first_not_of( " " );
  res[fp] = res[fp] - char(0x20); // first letter to caps...
  return res;
}

