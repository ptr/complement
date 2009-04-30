// -*- C++ -*- Time-stamp: <09/04/29 13:27:56 ptr>

/*
 * Copyright (c) 1997-1999, 2002-2003, 2005-2006, 2008-2009
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

__FIT_DECLSPEC
void __pack_base::__unpack( istream& s, string& str )
{
  uint32_t sz;
  s.read( (char *)&sz, sizeof(uint32_t) );
  sz = from_net( sz );
  str.erase();
  if ( sz > 0 ) {
    str.resize( sz );
    s.read( const_cast<char*>(str.data()), sz );
  }
}

__FIT_DECLSPEC
void __pack_base::__pack( ostream& s, const string& str )
{
  uint32_t sz = static_cast<uint32_t>( str.size() );
  sz = to_net( sz );
  s.write( (const char *)&sz, sizeof(uint32_t) );
  if ( !str.empty() ) {
    s.write( str.data(), str.size() );
  }
}

__FIT_DECLSPEC
void __pack_base::__unpack( std::istream& s, xmt::uuid_type& u )
{
  s.read( (char*)u.u.b, sizeof(u.u.b) );
}

__FIT_DECLSPEC
void __pack_base::__pack( std::ostream& s, const xmt::uuid_type& u )
{
  s.write( (const char*)u.u.b, sizeof(u.u.b) );
}

} // namespace stem
