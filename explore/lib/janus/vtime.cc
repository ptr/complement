// -*- C++ -*- Time-stamp: <09/10/12 13:28:36 ptr>

/*
 *
 * Copyright (c) 2008-2009
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
    EventHandler(),
    view( 0 ),
    lock_addr( stem::badaddr )
{
}

basic_vs::basic_vs( stem::addr_type id ) :
    EventHandler( id ),
    view( 0 ),
    lock_addr( stem::badaddr )
{
}

basic_vs::basic_vs( stem::addr_type id, const char* info ) :
    EventHandler( id, info ),
    view( 0 ),
    lock_addr( stem::badaddr )
{
}

basic_vs::basic_vs( const char* info ) :
    EventHandler( info ),
    view( 0 ),
    lock_addr( stem::badaddr )
{
}

basic_vs::~basic_vs()
{
  disable();

  stem::Event_base<basic_event> ev( VS_LEAVE );
  ev.value().view = view;

  vtime& self = vt[self_id()];

  for ( vtime::vtime_type::const_iterator i = self.vt.begin(); i != self.vt.end(); ++i ) {
    if ( i->first != self_id() ) {
      ev.dest( i->first );
      ev.value().vt = chg( self, vt[i->first] );
      // vt[i->first] = self;
      Send( ev );
    }
  }

  // well, VS_LEAVE may not departed yet...
  for ( access_container_type::iterator i = remotes_.begin(); i != remotes_.end(); ++i ) {
    delete *i;
  }
  remotes_.clear();
}

void basic_vs::vs( const stem::Event& inc_ev )
{
  if ( lock_addr != stem::badaddr ) {
    cerr << "View Update" << endl;
    return;
  }

  stem::Event_base<vs_event> ev( VS_EVENT );
  ev.value().view = view;
  ev.value().ev = inc_ev;
  ev.value().ev.setf( stem::__Event_Base::vs );

  vtime& self = vt[self_id()];
  ++self[self_id()];

  if ( (inc_ev.code() != VS_UPDATE_VIEW) && (inc_ev.code() != VS_LOCK_VIEW) ) {
    this->vs_event_origin( self, ev.value().ev );
  }

  // cerr << __FILE__ << ':' << __LINE__ << ' ' << self_id() << ' ' << hex << inc_ev.code() << dec << endl;
  for ( vtime::vtime_type::const_iterator i = self.vt.begin(); i != self.vt.end(); ++i ) {
    if ( i->first != self_id() ) {
      // cerr << __FILE__ << ':' << __LINE__ << ' ' << i->first << endl;
      ev.dest( i->first );
      ev.value().vt = self; // chg( self, vt[i->first] );
      // vt[i->first] = self; // <------------
      Send( ev );
    }
  }
  // cerr << __FILE__ << ':' << __LINE__ << ' ' << self_id() << endl;
}

// addr in network byte order, port in native byte order
void basic_vs::vs_tcp_point( uint32_t addr, int port )
{
  vs_points::access_t& p = points.insert( make_pair(self_id(), vs_points::access_t()) )->second;

  p.hostid = xmt::hostid();
  p.family = AF_INET;
  p.type = std::sock_base::sock_stream;
  p.data.resize( /* sizeof(uint32_t) + sizeof(uint16_t) */ 6 );
  memcpy( (void *)p.data.data(), (const void *)&addr, 4 );
  uint16_t prt = stem::to_net( static_cast<uint32_t>(port) );
  memcpy( (void *)(p.data.data() + 4), (const void *)&prt, 2 );
}

void basic_vs::vs_tcp_point( const sockaddr_in& a )
{
  if ( a.sin_family == PF_INET ) {
    vs_points::access_t& p = points.insert( make_pair(self_id(), vs_points::access_t()) )->second;

    p.hostid = xmt::hostid();
    p.family = AF_INET;
    p.type = std::sock_base::sock_stream;
    p.data.resize( /* sizeof(uint32_t) + sizeof(uint16_t) */ 6 );
    memcpy( (void *)p.data.data(), (const void *)&a.sin_addr, 4 );
    memcpy( (void *)(p.data.data() + 4), (const void *)&a.sin_port, 2 );
  }
}

void basic_vs::vs_join( const stem::addr_type& a )
{
  this->vs_pub_recover();

  stem::Event_base<vs_points> ev( VS_JOIN_RQ );

  ev.dest( a );
  ev.value().points = points;

  Send( ev );
}

void basic_vs::vs_join( const stem::addr_type& a, const char* host, int port )
{
  if ( !is_avail( a ) ) {
    remotes_.push_back( new stem::NetTransportMgr() );
    stem::addr_type trial_node = remotes_.back()->open( host, port );
    if ( remotes_.back()->is_open() && (trial_node != a) ) {
      remotes_.back()->add_route( a );
      remotes_.back()->add_remote_route( EventHandler::self_id() );
    }
  }
  vs_join( a );
}

void basic_vs::vs_join_request( const stem::Event_base<vs_points>& ev )
{
  // cerr << __FILE__ << ':' << __LINE__ << endl;
//  if ( leader == self_id() ) {
  stem::Event_base<vs_points> rsp( VS_JOIN_RS );

  rsp.value().view = view;
  rsp.value().points = points;
  rsp.dest( ev.src() );

  Send( rsp );

  for ( vs_points::points_type::const_iterator i = ev.value().points.begin(); i != ev.value().points.end(); ++i ) {
    if ( hostid() != i->second.hostid ) {
      if ( i->second.family == AF_INET ) {
        // i.e. IP not 127.x.x.x
        if ( *reinterpret_cast<const uint8_t*>(i->second.data.data()) != 127 ) {
          points.insert( *i );
        }
      }
    } else {
      points.insert( *i );
    }
  }

  if ( lock_addr == stem::badaddr ) {
    if ( vt[self_id()].vt.size() > 1 ) {
      // cerr << __FILE__ << ':' << __LINE__ << endl;
      lock_rsp.clear();
      group_applicant = ev.src();

      stem::EventVoid view_lock_ev( VS_LOCK_VIEW );

      basic_vs::vs( view_lock_ev );
      lock_addr = self_id(); // after vs()!
    } else { // single in group: lock not required
      // cerr << __FILE__ << ':' << __LINE__ << endl;
      ++view;
      vt[self_id()][ev.src()]; // i.e. create entry in vt
      stem::EventVoid update_view_ev( VS_UPDATE_VIEW );

      basic_vs::vs( update_view_ev );
    }
  }

//  } else {
//    ev.dest( leader );
//    Forward( ev );
//  }
}

void basic_vs::vs_group_points( const stem::Event_base<vs_points>& ev )
{
  // cerr << __FILE__ << ':' << __LINE__ << endl;

  // view = ev.value().view;

  for ( vs_points::points_type::const_iterator i = ev.value().points.begin(); i != ev.value().points.end(); ++i ) {
    if ( hostid() != i->second.hostid ) {
      if ( i->second.family == AF_INET ) {
        // i.e. IP not 127.x.x.x
        if ( *reinterpret_cast<const uint8_t*>(i->second.data.data()) != 127 ) {
          points.insert( *i );
          if ( !is_avail( i->first ) ) {
            for ( access_container_type::iterator j = remotes_.begin(); j != remotes_.end(); ) {
              if ( !(*j)->is_open() ) {
                delete *j;
                remotes_.erase( j++ );
              } else {
                ++j;
              }
            }

            remotes_.push_back( new stem::NetTransportMgr() );

            sockaddr_in sa;
            sa.sin_family = AF_INET;
            memcpy( (void*)&sa.sin_port, (void*)(i->second.data.data() + 4), 2 );
            memcpy( (void*)&sa.sin_addr.s_addr, (void *)i->second.data.data(), 4 );
            stem::addr_type trial_node = remotes_.back()->open( sa );
            if ( remotes_.back()->is_open() && (trial_node != i->first) ) {
              remotes_.back()->add_route( i->first );
              remotes_.back()->add_remote_route( EventHandler::self_id() );
            }
          }
        }
      }
    } else {
      points.insert( *i );
      if ( i->second.family == AF_INET ) {
        if ( !is_avail( i->first ) ) {
          for ( access_container_type::iterator j = remotes_.begin(); j != remotes_.end(); ) {
            if ( !(*j)->is_open() ) {
              delete *j;
              remotes_.erase( j++ );
            } else {
              ++j;
            }
          }
          remotes_.push_back( new stem::NetTransportMgr() );

          sockaddr_in sa;
          sa.sin_family = AF_INET;
          memcpy( (void*)&sa.sin_port, (void*)(i->second.data.data() + 4), 2 );
          memcpy( (void*)&sa.sin_addr.s_addr, (void *)i->second.data.data(), 4 );
          stem::addr_type trial_node = remotes_.back()->open( sa );
          if ( remotes_.back()->is_open() && (trial_node != i->first) ) {
            remotes_.back()->add_route( i->first );
            remotes_.back()->add_remote_route( EventHandler::self_id() );
          }
        }
      }
    }
  }
}

void basic_vs::vs_lock_view( const stem::EventVoid& ev )
{
  // cerr << __FILE__ << ':' << __LINE__ << endl;
  if ( lock_addr == stem::badaddr ) {
    // cerr << __FILE__ << ':' << __LINE__ << ' ' << ev.src() << endl;
    lock_addr = ev.src();
    stem::EventVoid view_lock_ev( VS_LOCK_VIEW_ACK );
    view_lock_ev.dest( lock_addr );
    Send( view_lock_ev );
  } else {
    // cerr << __FILE__ << ':' << __LINE__ << ' ' << ev.src() << endl;
    if ( ev.src() != lock_addr ) {
      if ( ev.src() < lock_addr ) {
        stem::EventVoid view_lock_ev_n( VS_LOCK_VIEW_NAK );
        view_lock_ev_n.dest( lock_addr );
        Send( view_lock_ev_n );

        lock_addr = ev.src();

        stem::EventVoid view_lock_ev( VS_LOCK_VIEW_ACK );
        view_lock_ev.dest( lock_addr );
        Send( view_lock_ev );
      } /* else {
      } */
    } /* else {
      stem::EventVoid view_lock_ev( VS_LOCK_VIEW_ACK );
      view_lock_ev.dest( lock_addr );
      Send( view_lock_ev );
    } */
  }
}

void basic_vs::vs_lock_view_ack( const stem::EventVoid& ev )
{
  // cerr << __FILE__ << ':' << __LINE__ << endl;
  if ( lock_addr == self_id() ) { // I'm owner of the lock
    lock_rsp.insert( ev.src() );
    vtime& self = vt[self_id()];
    // cerr << __FILE__ << ':' << __LINE__ << ' ' << lock_rsp.size() << ' ' << self.vt.size() << endl;
    if ( (lock_rsp.size() + 1) == self.vt.size() ) {
      // cerr << __FILE__ << ':' << __LINE__ << endl;
      for ( vtime::vtime_type::const_iterator i = self.vt.begin(); i !=self.vt.end(); ++i ) {
        if ( (i->first != self_id()) && (lock_rsp.find( i->first ) == lock_rsp.end()) ) {
          // cerr << __FILE__ << ':' << __LINE__ << endl;
          return;
        }
      }
      // cerr << __FILE__ << ':' << __LINE__ << ' ' << group_applicant << endl;
      // response from all group members available
      ++view;
      vt[self_id()][group_applicant]; // i.e. create entry in vt
      group_applicant = stem::badaddr;
      lock_addr = stem::badaddr; // before vs()!
      lock_rsp.clear();

      stem::EventVoid update_view_ev( VS_UPDATE_VIEW );

      basic_vs::vs( update_view_ev );      
    }
  }
}

void basic_vs::vs_lock_view_nak( const stem::EventVoid& ev )
{
  // cerr << __FILE__ << ':' << __LINE__ << endl;
  if ( lock_addr == self_id() ) { // I'm owner of the lock
    lock_addr = stem::badaddr;
    lock_rsp.clear();
  }
}

void basic_vs::vs_view_update()
{
  // cerr << __FILE__ << ':' << __LINE__ << endl;
  this->vs_pub_view_update();
}

void basic_vs::vt_process( const stem::Event_base<vs_event>& ev )
{
  // cerr << __FILE__ << ':' << __LINE__ << ' ' << self_id() << ' ' << hex << ev.value().ev.code() << dec << endl;
  // check the view version first:
  if ( ev.value().view != view ) {
    // cerr << __FILE__ << ':' << __LINE__ << endl;
    if ( ev.value().ev.code() != VS_UPDATE_VIEW ) {
      return; // ? view changed, but this object unknown yet
    }
    // cerr << __FILE__ << ':' << __LINE__ << endl;
    if ( (view != 0) && (lock_addr != ev.src()) ) {
      // cerr << __FILE__ << ':' << __LINE__ << ' ' << lock_addr 
      //      << ' ' << ev.src() << endl;
      return; // ? update view: not owner of lock
    }
    // cerr << __FILE__ << ':' << __LINE__ << endl;
    if ( (view != 0) && ((view + 1) != ev.value().view) ) {
      return; // ? view changed, but this object unknown yet
    }
    // cerr << __FILE__ << ':' << __LINE__ << ' ' << ev.value().vt.vt.size() << ' ' << self_id() << endl;
    if ( view == 0 ) {
      // cerr << __FILE__ << ':' << __LINE__ << ' ' << self_id() << endl;
      vt[self_id()] = ev.value().vt.vt; // align time with origin
      --vt[self_id()][ev.src()]; // will be incremented below
    }
    view = ev.value().view;
    lock_addr = stem::badaddr; // clear lock
  }
  // cerr << __FILE__ << ':' << __LINE__ << ' ' << self_id() << endl;

  // check virtual time:
  vtime tmp = vt[ev.src()];

  for ( vtime::vtime_type::const_iterator i = ev.value().vt.vt.begin(); i != ev.value().vt.vt.end(); ++i ) {
    if ( i->second < tmp[i->first] ) {
      // ev.src() fail?
      // cerr << __FILE__ << ':' << __LINE__ << ' ' << self_id() << endl;
      return;
    }
    tmp[i->first] = i->second;
  }

  // tmp.sup( ev.value().vt );

  vtime& self = vt[self_id()];

  for ( vtime::vtime_type::const_iterator i = self.vt.begin(); i != self.vt.end(); ++i ) {
    if ( i->first == ev.src() ) {
      if ( (i->second + 1) != tmp[ev.src()] ) {
        if ( (i->second + 1) < tmp[ev.src()] ) {
          // cerr << __FILE__ << ':' << __LINE__ << ' ' << i->first << ' ' << ev.src()  << ' ' << (i->second + 1) << ' ' << tmp[ev.src()] << endl;
          dc.push_back( ev ); // push event into delay queue
        } else {
          // Ghost event from past: Drop? Error?
          // cerr << __FILE__ << ':' << __LINE__ << ' ' << i->first << ' ' << ev.src() << endl;
        }
        return;
      }
    } else if ( i->second < tmp[i->first] ) {
      // cerr << __FILE__ << ':' << __LINE__ << ' ' << i->first << ' ' << ev.src() << ' ' << i->second << ' ' << tmp[i->first] << endl;
      dc.push_back( ev ); // push event into delay queue
      return;
    }
  }

  ++self[ev.src()];
  // cerr << __FILE__ << ':' << __LINE__ << endl;
  vt[ev.src()] = tmp; // vt[ev.src()] = self; ??   <--------
  
  ev.value().ev.src( ev.src() );
  ev.value().ev.dest( ev.dest() );

  if ( (ev.value().ev.code() != VS_UPDATE_VIEW) && (ev.value().ev.code() != VS_LOCK_VIEW) ) {
    // Update view not passed into vs_event_derivative,
    // it specific for Virtual Synchrony
    this->vs_event_derivative( self, ev.value().ev );
  } else {
    /* Specific for update view: vt[self_id()] should
       contain all group members, even if virtual time
       is zero (copy/assign vt don't copy entry with zero vtime!)
    */
    vtime& self = vt[self_id()];
    for ( vtime::vtime_type::const_iterator i = ev.value().vt.vt.begin(); i != ev.value().vt.vt.end(); ++i ) {
      self[i->first];
    }
  }

  // cerr << __FILE__ << ':' << __LINE__ << ' ' << self_id() << endl;
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

      for ( vtime::vtime_type::const_iterator i = k->value().vt.vt.begin(); i != k->value().vt.vt.end(); ++i ) {
        if ( i->second < tmp[i->first] ) {
          // ev.src() fail?
          dc.erase( k++ );
          goto try_next;
        }
        tmp[i->first] = i->second;
      }

      // tmp.sup( k->value().vt );

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

      if ( (k->value().ev.code() != VS_UPDATE_VIEW) && (k->value().ev.code() != VS_LOCK_VIEW) ) {
        // Update view not passed into vs_event_derivative,
        // it specific for Virtual Synchrony
        this->vs_event_derivative( self, k->value().ev );
      } else {
        /* Specific for update view: vt[self_id()] should
           contain all group members, even if virtual time
           is zero (copy/assign vt don't copy entry with zero vtime!)
        */
        vtime& self = vt[self_id()];
        for ( vtime::vtime_type::const_iterator i = k->value().vt.vt.begin(); i != k->value().vt.vt.end(); ++i ) {
          self[i->first];
        }
      }

      this->Dispatch( k->value().ev );

      dc.erase( k++ );
      delayed_process = true;

      try_next:
        ;
    }
  } while ( delayed_process );
}

void basic_vs::vs_leave( const stem::Event_base<basic_event>& ev )
{
  vt.erase( ev.src() );
  // view = ev.value().view;
  // points.erase( ev.src() );
  for ( vtime_matrix_type::iterator i = vt.begin(); i != vt.end(); ++i ) {
    i->second.vt.erase( ev.src() );
  }
}

void basic_vs::replay( const vtime& _vt, const stem::Event& inc_ev )
{
  // here must be: vt[self_id()] <= _vt;
  // another assume: replay called in correct order
  // (vs_event_origin + vs_event_derivative produce correct order)

//  if ( _vt <= vt[self_id()] ) {
//    throw std::runtime_error( "invalid VS event order for replay" );
//  }

//  vt[self_id()] = _vt;

  this->Dispatch( inc_ev );
}

const stem::code_type basic_vs::VS_EVENT         = 0x302;
const stem::code_type basic_vs::VS_LEAVE         = 0x303;
const stem::code_type basic_vs::VS_JOIN_RQ       = 0x304;
const stem::code_type basic_vs::VS_JOIN_RS       = 0x305;
const stem::code_type basic_vs::VS_LOCK_VIEW     = 0x306;
const stem::code_type basic_vs::VS_LOCK_VIEW_ACK = 0x307;
const stem::code_type basic_vs::VS_LOCK_VIEW_NAK = 0x308;
const stem::code_type basic_vs::VS_UPDATE_VIEW   = 0x309;

DEFINE_RESPONSE_TABLE( basic_vs )
  EV_Event_base_T_( ST_NULL, VS_EVENT, vt_process, vs_event )
  EV_Event_base_T_( ST_NULL, VS_LEAVE, vs_leave, basic_event )
  EV_Event_base_T_( ST_NULL, VS_JOIN_RQ, vs_join_request, vs_points )
  EV_Event_base_T_( ST_NULL, VS_JOIN_RS, vs_group_points, vs_points )
  EV_Event_base_T_( ST_NULL, VS_LOCK_VIEW, vs_lock_view, void )
  EV_Event_base_T_( ST_NULL, VS_LOCK_VIEW_ACK, vs_lock_view_ack, void )
  EV_Event_base_T_( ST_NULL, VS_LOCK_VIEW_NAK, vs_lock_view_nak, void )
  EV_VOID( ST_NULL, VS_UPDATE_VIEW, vs_view_update )
END_RESPONSE_TABLE

} // namespace janus
