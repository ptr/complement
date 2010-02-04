// -*- C++ -*- Time-stamp: <10/02/04 18:28:57 ptr>

/*
 *
 * Copyright (c) 2008-2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <janus/vtime.h>

#include <stdint.h>

#include <sys/socket.h>
#include <sockios/sockstream>
#include <stem/NetTransport.h>
#include <stem/Cron.h>
#include <stem/EDSEv.h>

#include <mt/mutex>

namespace janus {

using namespace std;
using namespace xmt;
using namespace stem;
using namespace std::tr2;

void vtime::pack( std::ostream& s ) const
{
  __pack( s, static_cast<uint8_t>(vt.size()) );
  for ( vtime_type::const_iterator i = vt.begin(); i != vt.end(); ++i ) {
    __pack( s, i->first );
    __pack( s, i->second );
  }
}

void vtime::unpack( std::istream& s )
{
  vt.clear();
  uint8_t n;
  __unpack( s, n );

  if ( !s.fail() && n != 0 ) {
    janus::addr_type addr;
    vtime_unit_type v;

    do {
      __unpack( s, addr );
      __unpack( s, v );

      if ( !s.fail() ) {
        vt[addr] = v;
      }
    } while ( --n != 0 && !s.fail() ); 
  }
}

bool vtime::operator <=( const vtime& r ) const
{
  if ( vt.empty() ) {
    return true;
  }

  for ( vtime_type::const_iterator i = vt.begin(); i != vt.end(); ++i ) {
    if ( i->second > 0 ) {
      vtime_type::const_iterator j = r.vt.find( i->first );
      if ( j == r.vt.end() || i->second > j->second ) {
	return false;
      }
    }
  }

  return true;
}

vtime vtime::operator -( const vtime& r ) const
{
  vtime tmp( r.vt.begin(), r.vt.end() );

  for ( vtime_type::iterator i = tmp.vt.begin(); i != tmp.vt.end(); ++i ) {
    if ( i->second > 0 ) {
      vtime_type::const_iterator p = vt.find(i->first);
      if ( p == vt.end() || p->second < i->second ) {
	throw range_error( "vtime different: right value grater then left" );
      }
      i->second = p->second - i->second;
    }
  }

  for ( vtime_type::const_iterator i = vt.begin(); i != vt.end(); ++i ) {
    vtime_type::iterator p = tmp.vt.find(i->first);
    if ( p == tmp.vt.end() ) {
      tmp.vt[i->first] = i->second;
    }
  }

  return tmp;
}

vtime vtime::operator +( const vtime& r ) const
{
  vtime tmp( vt.begin(), vt.end() );

  for ( vtime_type::const_iterator i = r.vt.begin(); i != r.vt.end(); ++i ) {
    tmp.vt[i->first] += i->second;
  }

  return tmp;
}

vtime& vtime::operator +=( const vtime& r )
{
  for ( vtime_type::const_iterator i = r.vt.begin(); i != r.vt.end(); ++i ) {
    vt[i->first] += i->second;
  }

  return *this;
}

#if 0
// template <>
vtime_type max( const vtime_type& l, const vtime_type& r )
{
  vtime_type tmp( l.begin(), l.end() );

  for ( vtime_type::const_iterator i = r.begin(); i != r.end(); ++i ) {
    tmp[i->first] = std::max( tmp[i->first], i->second );
  }
  return tmp;
}
#endif

#if 0
// template <>
vtime max( const vtime& l, const vtime& r )
{
  vtime tmp( l );

  for ( vtime_type::const_iterator i = r.vt.begin(); i != r.vt.end(); ++i ) {
    tmp[i->first] = std::max( tmp[i->first], i->second );
  }
  return tmp;
}
#endif

vtime& vtime::sup( const vtime& r )
{
  for ( vtime::vtime_type::const_iterator i = r.vt.begin(); i != r.vt.end(); ++i ) {
    vt[i->first] = std::max( vt[i->first], i->second );
  }
  return *this;
}

vtime& vtime::operator +=( const vtime::vtime_type::value_type& t )
{
  vt[t.first] += t.second;

  return *this;
}

// This assume that l >= r !!!
vtime chg( const vtime& l, const vtime& r )
{
  vtime tmp;

  for ( vtime::vtime_type::const_iterator i = l.vt.begin(); i != l.vt.end(); ++i ) {
    vtime::vtime_type::const_iterator j = r.vt.find( i->first );
    if ( j == r.vt.end() ) {
      tmp.vt.insert( *i );
    } else if ( i->second > j->second ) {
      tmp.vt.insert( *i );
    }
  }

  return tmp;
}

void vs_event::pack( std::ostream& s ) const
{
  basic_event::pack( s );
  __pack( s, ev.code() );
  __pack( s, ev.flags() );
  __pack( s, ev.value() );
}

void vs_event::unpack( std::istream& s )
{
  basic_event::unpack( s );
  stem::code_type c;
  __unpack( s, c );
  ev.code( c );
  uint32_t f;
  __unpack( s, f );
  ev.resetf( f );
  // string d;
  __unpack( s, ev.value() );
  // std::swap( d, ev.value() );
}

void vs_event::swap( vs_event& r )
{
  std::swap( vt, r.vt );
  std::swap( ev, r.ev );
}

void vs_points::pack( std::ostream& s ) const
{
  basic_event::pack( s );
  __pack( s, static_cast<uint32_t>( points.size() ) );
  for ( points_type::const_iterator i = points.begin(); i != points.end(); ++i ) {
    __pack( s, i->first );
    __pack( s, i->second.hostid );
    __pack( s, i->second.family );
    __pack( s, i->second.type );
    __pack( s, i->second.data );
  }
}

void vs_points::unpack( std::istream& s )
{
  basic_event::unpack( s );
  uint32_t sz;
  __unpack( s, sz );
  if ( !s.fail() ) {
    addr_type a;
    access_t acc;
    points.clear();
    while ( sz > 0 ) {
      __unpack( s, a );
      __unpack( s, acc.hostid );
      __unpack( s, acc.family );
      __unpack( s, acc.type );
      __unpack( s, acc.data );
      if ( !s.fail() ) {
        points.insert( make_pair( a, acc ) );
        --sz;
      } else {
        break;
      }
    }
  }
}

void vs_points::swap( vs_points& r )
{
  std::swap( vt, r.vt );
  std::swap( points, r.points );
}

void vs_join_rq::pack( std::ostream& s ) const
{
  vs_points::pack( s );
  __pack( s, reference );
}

void vs_join_rq::unpack( std::istream& s )
{
  vs_points::unpack( s );
  __unpack( s, reference );
}

void vs_join_rq::swap( vs_join_rq& r )
{
  vs_points::swap( r );
  std::swap( reference, r.reference );
}

void gvtime::pack( std::ostream& s ) const
{
  __pack( s, static_cast<uint8_t>(gvt.size()) );
  for ( gvtime_type::const_iterator i = gvt.begin(); i != gvt.end(); ++i ) {
    __pack( s, i->first );
    i->second.pack( s );
  }
}

void gvtime::unpack( std::istream& s )
{
  gvt.clear();
  uint8_t n;
  __unpack( s, n );
  while ( n-- > 0 ) {
    gid_type gid;
    __unpack( s, gid );
    gvt[gid].unpack( s );
  }
}

gvtime gvtime::operator -( const gvtime& r ) const
{
  gvtime tmp( r );

  for ( gvtime_type::iterator g = tmp.gvt.begin(); g != tmp.gvt.end(); ++g ) {
    gvtime_type::const_iterator i = gvt.find( g->first );
    if ( i == gvt.end() ) {
      throw range_error( "gvtime different: right value grater then left" );
    }

    g->second = i->second - g->second;
  }

  for ( gvtime_type::const_iterator g = gvt.begin(); g != gvt.end(); ++g ) {
    gvtime_type::const_iterator i = tmp.gvt.find(g->first);
    if ( i == tmp.gvt.end() ) {
      tmp.gvt[g->first] = g->second;
    }
  }

  return tmp;
}

gvtime& gvtime::operator +=( const gvtime_type::value_type& t )
{
  gvt[t.first] += t.second;

  return *this;  
}

gvtime& gvtime::operator +=( const gvtime& t )
{
  for ( gvtime_type::const_iterator g = t.gvt.begin(); g != t.gvt.end(); ++g ) {
    gvt[g->first] += g->second;
  }

  return *this;
}

} // namespace janus
