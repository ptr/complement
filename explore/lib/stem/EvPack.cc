// -*- C++ -*- Time-stamp: <06/07/20 11:17:10 ptr>

/*
 *
 * Copyright (c) 1997-1999, 2002, 2003, 2005
 * Petr Ovtchenkov
 *
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
 *
 * Licensed under the Academic Free License version 2.1
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

#include <config/feature.h>
#include "stem/EvPack.h"
#include <iterator>
#include <iostream>
#include <string>
#include <algorithm>
#include <stdint.h>

namespace stem {

using namespace std;

__FIT_DECLSPEC
void __pack_base::__net_unpack( istream& s, string& str )
{
  uint32_t sz;
  s.read( (char *)&sz, sizeof(uint32_t) );
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

__FIT_DECLSPEC
void __pack_base::__net_pack( ostream& s, const string& str )
{
  uint32_t sz = static_cast<uint32_t>( str.size() );
  sz = to_net( sz );
  s.write( (const char *)&sz, sizeof(uint32_t) );
#if !defined(__HP_aCC) || (__HP_aCC > 1) // linker problem, not compiler
  copy( str.begin(), str.end(), std::ostream_iterator<char,char,std::char_traits<char> >(s) );
#else // Mmm, may be with __STL_DEBUG should be a bit different...
// #  ifndef __STL_DEBUG
  copy( str.begin(), str.end(), ostream_iterator<char>(s) );
// #  else
//  __STD::__copy_aux(str.begin(), str.end(), __STD::ostream_iterator<char,char,__STD::char_traits<char> >(s), (char *)(0) );
// #  endif
#endif
}

__FIT_DECLSPEC
void __pack_base::__unpack( istream& s, string& str )
{
  string::size_type sz;
  s.read( (char *)&sz, sizeof(sz) );
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

__FIT_DECLSPEC
void __pack_base::__pack( ostream& s, const string& str )
{
  string::size_type sz = str.size();
  s.write( (const char *)&sz, sizeof(sz) );
#if !defined(__HP_aCC) || (__HP_aCC > 1) // linker problem, not compiler
  std::copy( str.begin(), str.end(), std::ostream_iterator<char,char,std::char_traits<char> >(s) );
#else // Mmm, may be with __STL_DEBUG should be a bit different...
// #  ifndef __STL_DEBUG
  copy( str.begin(), str.end(), ostream_iterator<char>(s) );
  // __STD::__copy_aux(str.begin(), str.end(), __STD::ostream_iterator<char,char,__STD::char_traits<char> >(s), (char *)(0) );
// #  else
//   __STD::__copy_aux(str.begin(), str.end(), __STD::ostream_iterator<char,char,__STD::char_traits<char> >(s), (char *)(0) );
//  for ( string::size_type i = 0; i < str.size(); ++i ) {
//    s << str[i];
//  }
// #  endif
#endif
}

} // namespace stem
