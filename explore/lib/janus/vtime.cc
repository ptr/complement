// -*- C++ -*- Time-stamp: <09/09/16 16:16:50 ptr>

/*
 *
 * Copyright (c) 2008-2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <janus/vtime.h>
// #include <janus/janus.h>
// #include <janus/vshostmgr.h>

#include <stdint.h>

namespace janus {

using namespace std;
using namespace xmt;
using namespace stem;

const janus::addr_type& nil_addr = xmt::nil_uuid;
const gid_type& nil_gid = xmt::nil_uuid;

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
  while ( n-- > 0 ) {
    janus::addr_type addr;
    vtime_unit_type v;

    __unpack( s, addr );
    __unpack( s, v );

    vt[addr] = v;
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

basic_vs::basic_vs() :
    EventHandler()
{
}

basic_vs::basic_vs( stem::addr_type id ) :
    EventHandler( id )
{
}

basic_vs::basic_vs( stem::addr_type id, const char* info ) :
    EventHandler( id, info )
{
}

basic_vs::basic_vs( const char* info ) :
    EventHandler( info )
{
}

basic_vs::~basic_vs()
{
}

void basic_vs::vs( const stem::Event& inc_ev )
{
  stem::Event_base<vs_event> ev( VS_EVENT );
  ev.value().ev = inc_ev;
  ev.value().ev.setf( stem::__Event_Base::vs );  

  vtime& self = vt[self_id()];
  ++self[self_id()];

  this->vs_event_origin( self, ev.value().ev );

  for ( vtime::vtime_type::const_iterator i = self.vt.begin(); i != self.vt.end(); ++i ) {
    if ( i->first != self_id() ) {
      ev.dest( i->first );
      ev.value().vt = chg( self, vt[i->first] );
      // vt[i->first] = self;
      Send( ev );
    }
  }
}

void basic_vs::new_member_round1( const stem::Event_base<VT_sync>& ev )
{
  vtime& newbie = vt[ev.src()];
  vtime& self = vt[self_id()];

  newbie = ev.value().vt;
  newbie[self_id()] = self[self_id()];
  self[ev.src()] = newbie[ev.src()];

  // super is ordered container
  group_members_type super;

  // all group members (except newbie) take from vtime
  for (vtime::vtime_type::const_iterator i = self.vt.begin(); i != self.vt.end(); ++i ) {
    super.insert( i->first );
  }

  group_members_type::const_iterator i = super.find( self_id() );

  if ( ++i == super.end() ) {
    i = super.begin();
  }

  if ( *i == self_id() ) {
    return; // I'm single in the group
  }

  if ( ev.src() != self_id() ) { // continue round 1
    stem::Event_base<VT_sync> ev_next( VS_GROUP_R1 );

    ev_next.dest( *i );
    ev_next.src( ev.src() );
    ev_next.value().vt = newbie;

    Forward( ev_next );
  } else { // start round 2
    stem::Event_base<VT_sync> ev_next( VS_GROUP_R2 );

    ev_next.dest( *i );
    ev_next.src( ev.src() );
    ev_next.value().vt = newbie;

    Forward( ev_next );
  }
}

void basic_vs::new_member_round2( const stem::Event_base<VT_sync>& ev )
{
  vtime& newbie = vt[ev.src()];
  vtime& self = vt[self_id()];

  newbie = ev.value().vt;

  // super is ordered container
  group_members_type super;

  // all group members (include newbie) take from vtime
  for (vtime::vtime_type::const_iterator i = self.vt.begin(); i != self.vt.end(); ++i ) {
    super.insert( i->first );
  }

  group_members_type::const_iterator i = super.find( self_id() );

  if ( ++i == super.end() ) {
    i = super.begin();
  }

  if ( *i == self_id() ) {
    return; // I'm single in the group?
  }

  if ( ev.src() != self_id() ) {
    ev.dest( *i );
    Forward( ev );
  } else {
    for ( vtime::vtime_type::const_iterator i = self.vt.begin(); i != self.vt.end(); ++i ) {
      if ( i->first != self_id() ) {
        vt[i->first][i->first] = i->second;
        vt[i->first][self_id()] = self[self_id()];
      }      
    }
    this->round2_pass();
  }
}

void basic_vs::vt_process( const stem::Event_base<vs_event>& ev )
{
  vtime tmp = vt[ev.src()];
  tmp.sup( ev.value().vt );

  vtime& self = vt[self_id()];

  for ( vtime::vtime_type::const_iterator i = self.vt.begin(); i != self.vt.end(); ++i ) {
    if ( i->first == ev.src() ) {
      if ( (i->second + 1) != tmp[ev.src()] ) {
        if ( (i->second + 1) < tmp[ev.src()] ) {
          dc.push_back( ev ); // Delay
        } else {
          // Ghost event from past: Drop? Error?
        }
        return;
      }
    } else if ( i->second < tmp[i->first] ) {
      dc.push_back( ev ); // Delay
      return;
    }
  }

  ++self[ev.src()];
  vt[ev.src()] = tmp; // vt[ev.src()] = self; ??
  
  ev.value().ev.src( ev.src() );
  ev.value().ev.dest( ev.dest() );

  this->vs_event_derivative( self, ev.value().ev );

  this->Dispatch( ev.value().ev );

  /*
    for each event in delay_queue try to process it;
    repeat procedure if any event from delay_queue was processed.
   */
  bool delayed_process;
  do {
    delayed_process = false;
    for ( delay_container_type::iterator k = dc.begin(); k != dc.end(); ) {
      tmp = vt[k->src()];
      tmp.sup( k->value().vt );

      for ( vtime::vtime_type::const_iterator i = self.vt.begin(); i != self.vt.end(); ++i ) {
        if ( i->first == k->src() ) {
          if ( (i->second + 1) != tmp[k->src()] ) {
            ++k;
            goto try_next;
          }
        } else if ( i->second < tmp[i->first] ) {
          ++k;
          goto try_next;
        }
      }

      ++self[k->src()];
      vt[k->src()] = tmp; // vt[k->src()] = self; ??

      k->value().ev.src( k->src() );
      k->value().ev.dest( k->dest() );

      this->vs_event_derivative( self, k->value().ev );

      this->Dispatch( k->value().ev );

      dc.erase( k++ );
      delayed_process = true;

      try_next:
        ;
    }
  } while ( delayed_process );
}

const stem::code_type basic_vs::VS_GROUP_R1 = 0x300;
const stem::code_type basic_vs::VS_GROUP_R2 = 0x301;
const stem::code_type basic_vs::VS_EVENT = 0x302;

DEFINE_RESPONSE_TABLE( basic_vs )
  EV_Event_base_T_( ST_NULL, VS_GROUP_R1, new_member_round1, VT_sync )
  EV_Event_base_T_( ST_NULL, VS_GROUP_R2, new_member_round2, VT_sync )
  EV_Event_base_T_( ST_NULL, VS_EVENT, vt_process, vs_event )
END_RESPONSE_TABLE

} // namespace janus
