// -*- C++ -*- Time-stamp: <99/09/22 10:31:37 ptr>

/*
 *
 * Copyright (c) 1996-1998
 * Petr Ovchenkov
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

#ifndef __CyrMoney_h
#define __CyrMoney_h

#ident "$SunId$ %Q%"

#ifndef __config_feature_h
#include <config/feature.h>
#endif

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
