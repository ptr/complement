// -*- C++ -*- Time-stamp: <2011-03-14 17:14:48 ptr>

/*
 * Copyright (c) 1997-1999, 2002-2003, 2005-2006, 2008-2011
 * Petr Ovtchenkov
 *
 * Copyright (c) 1999-2001
 * ParallelGraphics Ltd.
 *
 * Licensed under the Academic Free License version 3.0
 * 
 */

#include <config/feature.h>
#include "stem/Event.h"
#include <iterator>
#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <stdint.h>
#include <cstring>

namespace stem {

using namespace std;

void __pack_base::__unpack( istream& s, string& str )
{
  uint32_t sz;
  s.read( (char *)&sz, sizeof(uint32_t) );
  if ( !s.fail() ) {
    sz = from_net( sz );
    str.erase();
    if ( sz > 0 ) {
      str.reserve( sz );
      copy_n( istreambuf_iterator<char>(s), sz, back_inserter(str) );
    }
  }
}

void __pack_base::__pack( ostream& s, const string& str )
{
  uint32_t sz = static_cast<uint32_t>( str.size() );
  sz = to_net( sz );
  s.write( (const char *)&sz, sizeof(uint32_t) );
  if ( !str.empty() ) {
    s.write( str.data(), str.size() );
  }
}

void __pack_base::__unpack( std::istream& s, xmt::uuid_type& u )
{
  s.read( reinterpret_cast<char*>(u.u.b), sizeof(u.u.b) );
}

void __pack_base::__pack( std::ostream& s, const xmt::uuid_type& u )
{
  s.write( reinterpret_cast<const char*>(u.u.b), sizeof(u.u.b) );
}

} // namespace stem
