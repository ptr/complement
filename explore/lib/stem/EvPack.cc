// -*- C++ -*- Time-stamp: <00/04/04 19:41:14 ptr>

/*
 *
 * Copyright (c) 1997-1999
 * Petr Ovchenkov
 *
 * Copyright (c) 1999-2000
 * ParallelGraphics
 
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 */

#ident "$SunId$ %Q%"

#include <config/feature.h>
#include "EDS/EvPack.h"
#include <iterator>
#include <string>

namespace EDS {

using __STD::string;
using __STD::istream;
using __STD::ostream;
using __STD::copy;

__PG_DECLSPEC
void __pack_base::__net_unpack( istream& s, string& str )
{
  string::size_type sz;
  s.read( (char *)&sz, 4 );
  sz = from_net( sz );
  if ( sz > 0 ) {
    str.erase();
    str.reserve( sz );
    while ( sz-- > 0 ) {
      str += (char)s.get();
    }
  }
}

__PG_DECLSPEC
void __pack_base::__net_pack( ostream& s, const string& str )
{
  string::size_type sz = str.size();
  sz = to_net( sz );
  s.write( (const char *)&sz, 4 );
  copy( str.begin(), str.end(), __STD::ostream_iterator<char,char,__STD::char_traits<char> >(s) );
}

__PG_DECLSPEC
void __pack_base::__unpack( istream& s, string& str )
{
  string::size_type sz;
  s.read( (char *)&sz, 4 );
  if ( sz > 0 ) {
    str.erase();
    str.reserve( sz );
    while ( sz-- > 0 ) {
      str += (char)s.get();
    }
  }
}

__PG_DECLSPEC
void __pack_base::__pack( ostream& s, const string& str )
{
  string::size_type sz = str.size();
  s.write( (const char *)&sz, 4 );
  copy( str.begin(), str.end(), __STD::ostream_iterator<char,char,__STD::char_traits<char> >(s) );
}

} // namespace EDS
