// -*- C++ -*- Time-stamp: <07/08/03 09:22:05 ptr>

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
#include "stem/Event.h"
#include <iterator>
#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <stdint.h>

namespace std {

ostream& operator <<( ostream& o, const stem::gaddr_type& g )
{
  ios_base::fmtflags f = o.flags( ios_base::hex );

  o << setfill( '0' ) 
    << setw(8) << g.hid.u.l[0]
    << setw(8) << g.hid.u.l[1]
    << '-' << dec << g.pid << '-'
    << hex << setfill( '0' ) << setw(8) << g.addr;

  o.flags( f );

  return o;
}

} // namespace std

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

__FIT_DECLSPEC void gaddr_type::pack( std::ostream& s ) const
{
  s.write( (const char *)hid.u.b, 16 );
  __pack( s, pid );
  __pack( s, addr );
}

__FIT_DECLSPEC void gaddr_type::unpack( std::istream& s )
{
  s.read( (char *)hid.u.b, 16 );
  __unpack( s, pid );
  __unpack( s, addr );
}

__FIT_DECLSPEC void gaddr_type::net_pack( std::ostream& s ) const
{
  s.write( (const char *)hid.u.b, 16 );
  __net_pack( s, pid );
  __net_pack( s, addr );
}

__FIT_DECLSPEC void gaddr_type::net_unpack( std::istream& s )
{
  s.read( (char *)hid.u.b, 16 );
  __net_unpack( s, pid );
  __net_unpack( s, addr );
}

__FIT_DECLSPEC void gaddr_type::_xnet_pack( char *buf ) const
{
  uint64_t _pid = to_net( pid );
  addr_type _addr = to_net( addr );

  // copy( (char *)hid.u.b, (char *)hid.u.b + 16, buf );
  memcpy( (void *)buf, (const void *)hid.u.b, 16 );
  memcpy( (void *)(buf + 16), &_pid, sizeof(pid) );
  memcpy( (void *)(buf + 16 + sizeof(pid)), &_addr, sizeof(addr_type) );
}

__FIT_DECLSPEC void gaddr_type::_xnet_unpack( const char *buf )
{
  memcpy( (void *)hid.u.b, (const void *)buf, 16 );
  memcpy( (void *)&pid, (const void *)(buf + 16), sizeof(pid) );
  memcpy( (void *)&addr, (const void *)(buf + 16 + sizeof(pid)), sizeof(addr_type) );
  pid = from_net( pid );
  addr = from_net( addr );
}

__FIT_DECLSPEC bool gaddr_type::operator <( const gaddr_type& ga ) const
{
  if ( hid < ga.hid ) {
    return true;
  } else if ( ga.hid < hid ) {
    return false;
  }
  if ( pid < ga.pid ) {
    return true;
  } else if ( ga.pid < pid ) {
    return false;
  }
  if ( addr < ga.addr ) {
    return true;
  }
  return false;
}

} // namespace stem
