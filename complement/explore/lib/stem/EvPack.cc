// -*- C++ -*- Time-stamp: <06/10/04 11:13:07 ptr>

/*
 * Copyright (c) 1997-1999, 2002, 2003, 2005, 2006
 * Petr Ovtchenkov
 *
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 * 
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
  str.erase();
  if ( sz > 0 ) {
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
  copy( str.begin(), str.end(), ostream_iterator<char>(s) );
#endif
}

__FIT_DECLSPEC
void __pack_base::__unpack( istream& s, string& str )
{
  string::size_type sz;
  s.read( (char *)&sz, sizeof(sz) );
  str.erase();
  if ( sz > 0 ) {
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
  copy( str.begin(), str.end(), ostream_iterator<char>(s) );
#endif
}

} // namespace stem