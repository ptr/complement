// -*- C++ -*- Time-stamp: <00/09/12 14:07:11 ptr>

/*
 *
 * Copyright (c) 1997-1999
 * Petr Ovchenkov
 *
 * Copyright (c) 1999-2000
 * ParallelGraphics Ltd.
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

#ifdef __unix
#  ifdef __HP_aCC
#pragma VERSIONID "$SunId$"
#  else
#pragma ident "$SunId$"
#  endif
#endif

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
#if defined(_MSC_VER) && (_MSC_VER < 1200)
    char ch;
#endif
    while ( sz-- > 0 ) {
#if defined(_MSC_VER) && (_MSC_VER < 1200)
      s.get( ch );
      str += ch;
#else
      str += (char)s.get();
#endif
    }
  }
}

__PG_DECLSPEC
void __pack_base::__net_pack( ostream& s, const string& str )
{
  string::size_type sz = str.size();
  sz = to_net( sz );
  s.write( (const char *)&sz, 4 );
#if !defined(__HP_aCC) || (__HP_aCC > 1)
  copy( str.begin(), str.end(), __STD::ostream_iterator<char,char,__STD::char_traits<char> >(s) );
#else // Mmm, may be with __STL_DEBUG should be a bit different...
  __STD::__copy_aux(str.begin(), str.end(), __STD::ostream_iterator<char,char,__STD::char_traits<char> >(s), (char *)(0) );
#endif
}

__PG_DECLSPEC
void __pack_base::__unpack( istream& s, string& str )
{
  string::size_type sz;
  s.read( (char *)&sz, 4 );
  if ( sz > 0 ) {
    str.erase();
    str.reserve( sz );
#if defined(_MSC_VER) && (_MSC_VER < 1200)
    char ch;
#endif
    while ( sz-- > 0 ) {
#if defined(_MSC_VER) && (_MSC_VER < 1200)
      s.get( ch );
      str += ch;
#else
      str += (char)s.get();
#endif
    }
  }
}

__PG_DECLSPEC
void __pack_base::__pack( ostream& s, const string& str )
{
  string::size_type sz = str.size();
  s.write( (const char *)&sz, 4 );
#if !defined(__HP_aCC) || (__HP_aCC > 1)
  __STD::copy( str.begin(), str.end(), __STD::ostream_iterator<char,char,__STD::char_traits<char> >(s) );
#else // Mmm, may be with __STL_DEBUG should be a bit different...
  __STD::__copy_aux(str.begin(), str.end(), __STD::ostream_iterator<char,char,__STD::char_traits<char> >(s), (char *)(0) );
#endif
}

} // namespace EDS
