// -*- C++ -*- Time-stamp: <10/06/28 19:18:38 ptr>

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

namespace detail {

void access_points::pack( std::ostream& s ) const
{
  __pack( s, static_cast<uint32_t>( points.size() ) );
  for (  janus::vs_points::points_type::const_iterator i = points.begin(); i != points.end(); ++i ) {
    __pack( s, i->first );
    __pack( s, i->second.hostid );
    __pack( s, i->second.family );
    __pack( s, i->second.type );
    __pack( s, i->second.data );
  }
}

void access_points::unpack( std::istream& s )
{
  uint32_t sz;
  __unpack( s, sz );
  if ( !s.fail() ) {
    addr_type a;
    janus::vs_points::access_t acc;
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

void access_points::swap( access_points& r )
{
  std::swap( points, r.points );
}

} // namespace detail

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

void vs_event_total_order::pack( std::ostream& s ) const
{
  // basic_event::pack( s );
  __pack( s, id );
  __pack( s, ev.code() );
  __pack( s, ev.flags() );
  __pack( s, ev.value() );
}

void vs_event_total_order::unpack( std::istream& s )
{
  // basic_event::unpack( s );
  __unpack( s, id );

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

void vs_event_total_order::swap( vs_event_total_order& r )
{
  // std::swap( vt, r.vt );
  std::swap( id, r.id );
  std::swap( ev, r.ev );
}

} // namespace janus
