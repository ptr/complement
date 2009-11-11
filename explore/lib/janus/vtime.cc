// -*- C++ -*- Time-stamp: <09/11/11 23:56:38 ptr>

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
#include <stem/Cron.h>
#include <stem/EDSEv.h>

#include <mt/mutex>

namespace janus {

using namespace std;
using namespace xmt;
using namespace stem;
using namespace std::tr2;

const janus::addr_type& nil_addr = xmt::nil_uuid;
const gid_type& nil_gid = xmt::nil_uuid;

static std::tr2::milliseconds vs_lock_timeout( 200 );

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

static char* Init_buf[128];
Cron* basic_vs::_cron = 0;

static int *_rcount = 0;
#if 1 // depends where fork happens: in the EvManager loop (stack) or not.
void basic_vs::Init::__at_fork_prepare()
{
}

void basic_vs::Init::__at_fork_child()
{
  if ( *_rcount != 0 ) {
    basic_vs::_cron->~Cron();
    basic_vs::_cron = new( basic_vs::_cron ) Cron();
  }
}

void basic_vs::Init::__at_fork_parent()
{
}
#endif

void basic_vs::Init::_guard( int direction )
{
  static recursive_mutex _init_lock;

  lock_guard<recursive_mutex> lk(_init_lock);
  static int _count = 0;

  if ( direction ) {
    if ( _count++ == 0 ) {
#ifdef _PTHREADS
      _rcount = &_count;
      pthread_atfork( __at_fork_prepare, __at_fork_parent, __at_fork_child );
#endif
      basic_vs::_cron = new Cron();
      basic_vs::_cron->enable();
    }
  } else {
    --_count;
    if ( _count == 0 ) {
      basic_vs::_cron->disable();
      delete basic_vs::_cron;
      basic_vs::_cron = 0;
    }
  }
}

basic_vs::Init::Init()
{ _guard( 1 ); }

basic_vs::Init::~Init()
{ _guard( 0 ); }

basic_vs::basic_vs() :
    EventHandler(),
    view( 0 ),
    lock_addr( stem::badaddr )
{
  new( Init_buf ) Init();
}

basic_vs::basic_vs( stem::addr_type id ) :
    EventHandler( id ),
    view( 0 ),
    lock_addr( stem::badaddr )
{
  new( Init_buf ) Init();
}

basic_vs::basic_vs( stem::addr_type id, const char* info ) :
    EventHandler( id, info ),
    view( 0 ),
    lock_addr( stem::badaddr )
{
  new( Init_buf ) Init();
}

basic_vs::basic_vs( const char* info ) :
    EventHandler( info ),
    view( 0 ),
    lock_addr( stem::badaddr )
{
  new( Init_buf ) Init();
}

basic_vs::~basic_vs()
{
  disable();

  ((Init *)Init_buf)->~Init();

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

  const stem::code_type code = inc_ev.code();

  if ( (code != VS_UPDATE_VIEW) && (code != VS_LOCK_VIEW) && (code != VS_FLUSH_LOCK_VIEW ) ) {
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

void basic_vs::send_to_vsg( const stem::Event& ev ) // not VS!
{
  stem::Event sev = ev;

  sev.src( self_id() );

  vtime& self = vt[self_id()];

  for ( vtime::vtime_type::const_iterator i = self.vt.begin(); i != self.vt.end(); ++i ) {
    if ( i->first != self_id() ) {
      // cerr << __FILE__ << ':' << __LINE__ << ' ' << i->first << endl;
      sev.dest( i->first );
      Forward( sev );
    }
  }
}

void basic_vs::forward_to_vsg( const stem::Event& ev ) // not VS!
{
  stem::Event sev = ev;

  vtime& self = vt[self_id()];

  for ( vtime::vtime_type::const_iterator i = self.vt.begin(); i != self.vt.end(); ++i ) {
    if ( i->first != self_id() ) {
      // cerr << __FILE__ << ':' << __LINE__ << ' ' << i->first << endl;
      sev.dest( i->first );
      Forward( sev );
    }
  }
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

int basic_vs::vs_join( const stem::addr_type& a )
{
  PushState( VS_ST_LOCKED );

  xmt::uuid_type ref = this->vs_pub_recover();

  if ( is_avail( a ) ) {
    stem::Event_base<vs_join_rq> ev( VS_JOIN_RQ );

    ev.dest( a );
    ev.value().points = points;
    ev.value().reference = ref;

    Send( ev );

    add_lock_safety();  // belay: avoid infinite lock

    return 0;
  }
  // peer unavailable (or I'm group founder), no lock required
  PopState( VS_ST_LOCKED );

  return a == stem::badaddr ? 0 : 1;
}

int basic_vs::vs_join( const stem::addr_type& a, const char* host, int port )
{
  if ( !is_avail( a ) ) {
    if ( a == stem::badaddr ) {
      return 2;
    }

    remotes_.push_back( new stem::NetTransportMgr() );
    stem::addr_type trial_node = remotes_.back()->open( host, port );

    if ( !remotes_.back()->is_open() ) {
      delete remotes_.back();
      remotes_.pop_back();

      return 3;
    }

    if ( trial_node != a ) {
      remotes_.back()->add_route( a );
    }
    remotes_.back()->add_remote_route( EventHandler::self_id() );
  }

  return vs_join( a );
}

int basic_vs::vs_join( const stem::addr_type& a, const sockaddr_in& srv )
{
  if ( !is_avail( a ) ) {
    if ( a == stem::badaddr ) {
      return 2;
    }

    remotes_.push_back( new stem::NetTransportMgr() );
    stem::addr_type trial_node = remotes_.back()->open( srv );

    if ( !remotes_.back()->is_open() ) {
      delete remotes_.back();
      remotes_.pop_back();

      return 3;
    }

    if ( trial_node != a ) {
      remotes_.back()->add_route( a );
    }
    remotes_.back()->add_remote_route( EventHandler::self_id() );
  }

  return vs_join( a );
}

int basic_vs::vs_join( const char* host, int port )
{
  remotes_.push_back( new stem::NetTransportMgr() );
  stem::addr_type trial_node = remotes_.back()->open( host, port );

  if ( !remotes_.back()->is_open() ) {
    delete remotes_.back();
    remotes_.pop_back();

    return 3;
  }

  if ( trial_node == stem::badaddr ) {
    delete remotes_.back();
    remotes_.pop_back();

    return 4;
  }

  remotes_.back()->add_route( trial_node );
  remotes_.back()->add_remote_route( EventHandler::self_id() );

  return vs_join( trial_node );
}

int basic_vs::vs_join( const sockaddr_in& a )
{
  remotes_.push_back( new stem::NetTransportMgr() );
  stem::addr_type trial_node = remotes_.back()->open( a );

  if ( !remotes_.back()->is_open() ) {
    delete remotes_.back();
    remotes_.pop_back();

    return 3;
  }

  if ( trial_node == stem::badaddr ) {
    delete remotes_.back();
    remotes_.pop_back();

    return 4;
  }

  remotes_.back()->add_route( trial_node );
  remotes_.back()->add_remote_route( EventHandler::self_id() );

  return vs_join( trial_node );
}

basic_vs::size_type basic_vs::vs_group_size() const
{
  vtime_matrix_type::const_iterator i = vt.find( self_id() );
  
  return i == vt.end() ? 0 : i->second.vt.size();
}

void basic_vs::vs_join_request( const stem::Event_base<vs_join_rq>& ev )
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
    vtime& self = vt[self_id()];

    if ( self.vt.size() > 1 ) {
      // cerr << __FILE__ << ':' << __LINE__ << endl;
      lock_rsp.clear();
      group_applicant = ev.src();

      // check: group_applicant re-enter (fail was not detected yet)
      for ( vtime::vtime_type::iterator i = self.vt.begin(); i != self.vt.end(); ++i ) {
        if ( i->first == group_applicant ) { // same address
          vt.erase( group_applicant );
          self.vt.erase( i );
          break;
        }
      }

      stem::EventVoid view_lock_ev( VS_LOCK_VIEW );

      basic_vs::vs( view_lock_ev );
      lock_addr = self_id(); // after vs()!
      PushState( VS_ST_LOCKED );
      this->vs_resend_from( ev.value().reference, ev.src() );

      add_lock_safety(); // belay: avoid infinite lock
    } else { // single in group: lock not required
      // cerr << __FILE__ << ':' << __LINE__ << endl;
      this->vs_resend_from( ev.value().reference, ev.src() );

      ++view;
      self[ev.src()]; // i.e. create entry in vt
      stem::EventVoid update_view_ev( VS_UPDATE_VIEW );

      basic_vs::vs( update_view_ev );
    }
  }

//  } else {
//    ev.dest( leader );
//    Forward( ev );
//  }
}

void basic_vs::vs_join_request_lk( const stem::Event_base<vs_join_rq>& ev )
{
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
  // cerr << __FILE__ << ':' << __LINE__ << ' ' << ev.src() << ' ' << self_id() << endl;
  PushState( VS_ST_LOCKED );
  lock_addr = ev.src();
  stem::EventVoid view_lock_ev( VS_LOCK_VIEW_ACK );
  view_lock_ev.dest( lock_addr );
  Send( view_lock_ev );

  add_lock_safety(); // belay: avoid infinite lock
}

void basic_vs::vs_lock_view_lk( const stem::EventVoid& ev )
{
  // cerr << __FILE__ << ':' << __LINE__ << ' ' << ev.src() << ' ' << self_id() << endl;
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
      PopState( VS_ST_LOCKED );
      lock_rsp.clear();

      rm_lock_safety();

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
    PopState( VS_ST_LOCKED );

    rm_lock_safety();
  }
}

void basic_vs::vs_view_update()
{
  lock_addr = stem::badaddr; // clear lock
  PopState( VS_ST_LOCKED );

  rm_lock_safety();

  // cerr << __FILE__ << ':' << __LINE__ << endl;
  this->vs_pub_view_update();
}

void basic_vs::vs_process( const stem::Event_base<vs_event>& ev )
{
  // cerr << __FILE__ << ':' << __LINE__ << ' ' << self_id() << ' ' << hex << ev.value().ev.code() << dec << endl;
  // check the view version first:
  if ( ev.value().view != view ) {
    if ( ev.value().view > view ) {
      dc.push_back( ev ); // push event into delay queue
    }
    return;
  }

  stem::code_type code = ev.value().ev.code();

  if ( code == VS_UPDATE_VIEW ) {
    dc.push_back( ev ); // push event into delay queue
    return;
  }

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
  vt[ev.src()] = tmp; // vt[ev.src()] = self; ??   <--------
  
  ev.value().ev.src( ev.src() );
  ev.value().ev.dest( ev.dest() );

  if ( (code != VS_LOCK_VIEW) && (code != VS_FLUSH_LOCK_VIEW) ) {
    // Update view not passed into vs_event_derivative,
    // it specific for Virtual Synchrony
    this->vs_event_derivative( self, ev.value().ev );
  }

  this->Dispatch( ev.value().ev );

  if ( !dc.empty() ) {
    process_delayed();
  }
}

void basic_vs::vs_process_lk( const stem::Event_base<vs_event>& ev )
{
  const stem::code_type code = ev.value().ev.code();

  if ( ev.value().view != view ) {
    // cerr << __FILE__ << ':' << __LINE__ << endl;
    if ( code != VS_UPDATE_VIEW ) {
      dc.push_back( ev ); // push event into delay queue
      return; // ? view changed, but this object unknown yet
    }
//    cerr << __FILE__ << ':' << __LINE__ << ' ' << ev.src() << ' ' << self_id() << endl;
    if ( (view != 0) && (lock_addr != ev.src()) ) {
      // cerr << __FILE__ << ':' << __LINE__ << ' ' << lock_addr 
      //      << ' ' << ev.src() << endl;
      return; // ? update view: not owner of lock
    }
    // cerr << __FILE__ << ':' << __LINE__ << endl;
    if ( (view != 0) && ((view + 1) != ev.value().view) ) {
      dc.push_back( ev ); // push event into delay queue
      return; // ? view changed, but this object unknown yet
    }
    // cerr << __FILE__ << ':' << __LINE__ << ' ' << ev.value().vt.vt.size() << ' ' << self_id() << endl;
    if ( view == 0 ) {
      // cerr << __FILE__ << ':' << __LINE__ << ' ' << self_id() << endl;
      vt[self_id()] = ev.value().vt.vt; // align time with origin
      --vt[self_id()][ev.src()]; // will be incremented below
    }
    view = ev.value().view;
  }

  if ( (code != VS_UPDATE_VIEW) && (code != VS_LOCK_VIEW) && (code != VS_FLUSH_VIEW) && (code != VS_FLUSH_LOCK_VIEW) ) {
    dc.push_back( ev ); // push event into delay queue
    return;
  }

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
  vt[ev.src()] = tmp; // vt[ev.src()] = self; ??   <--------
  
  ev.value().ev.src( ev.src() );
  ev.value().ev.dest( ev.dest() );

  if ( code == VS_UPDATE_VIEW ) {
    /* Specific for update view: vt[self_id()] should
       contain all group members, even if virtual time
       is zero (copy/assign vt don't copy entry with zero vtime!)
    */
    for ( vtime::vtime_type::const_iterator i = ev.value().vt.vt.begin(); i != ev.value().vt.vt.end(); ++i ) {
      self[i->first];
    }
  } else if ( code == VS_FLUSH_VIEW ) {
    // flush passed into vs_event_derivative
    this->vs_event_derivative( self, ev.value().ev );
  }

  this->Dispatch( ev.value().ev );

  if ( !dc.empty() ) {
    process_delayed();
  }
}

void basic_vs::process_delayed()
{
  /*
    for each event in delay_queue try to process it;
    repeat procedure if any event from delay_queue was processed.
   */
  bool delayed_process;
  vtime tmp;
  vtime& self = vt[self_id()];

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

void basic_vs::replay( const vtime& _vt, const stem::Event& inc_ev )
{
  // here must be: vt[self_id()] <= _vt;
  // another assume: replay called in correct order
  // (vs_event_origin + vs_event_derivative produce correct order)

//  if ( _vt <= vt[self_id()] ) {
//    throw std::runtime_error( "invalid VS event order for replay" );
//  }

//  vt[self_id()] = _vt;

  if ( inc_ev.code() != VS_FLUSH_VIEW ) {
    this->Dispatch( inc_ev );
  }
}

void basic_vs::vs_send_flush()
{
  if ( lock_addr == stem::badaddr ) {
    if ( vt[self_id()].vt.size() > 1 ) {
      // cerr << __FILE__ << ':' << __LINE__ << endl;
      lock_rsp.clear();

      stem::EventVoid view_lock_ev( VS_FLUSH_LOCK_VIEW );

      basic_vs::vs( view_lock_ev );
      lock_addr = self_id(); // after vs()!
      PushState( VS_ST_LOCKED );

      add_lock_safety(); // belay: avoid infinite lock
    } else { // single in group: lock not required
      // cerr << __FILE__ << ':' << __LINE__ << endl;
      stem::Event_base<xmt::uuid_type> flush_ev( VS_FLUSH_VIEW );

      flush_ev.value() = xmt::uid();

      basic_vs::vs( flush_ev );
    }
  }
}

void basic_vs::vs_flush_lock_view( const stem::EventVoid& ev )
{
  PushState( VS_ST_LOCKED );
  lock_addr = ev.src();
  stem::EventVoid view_lock_ev( VS_FLUSH_LOCK_VIEW_ACK );
  view_lock_ev.dest( lock_addr );
  Send( view_lock_ev );

  add_lock_safety(); // belay: avoid infinite lock
}

void basic_vs::vs_flush_lock_view_lk( const stem::EventVoid& ev )
{
  if ( ev.src() != lock_addr ) {
    if ( ev.src() < lock_addr ) {
      stem::EventVoid view_lock_ev_n( VS_FLUSH_LOCK_VIEW_NAK );
      view_lock_ev_n.dest( lock_addr );
      Send( view_lock_ev_n );

      lock_addr = ev.src();

      stem::EventVoid view_lock_ev( VS_FLUSH_LOCK_VIEW_ACK );
      view_lock_ev.dest( lock_addr );
      Send( view_lock_ev );
    } /* else {
    } */
  }
}

void basic_vs::vs_flush_lock_view_ack( const stem::EventVoid& ev )
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
      lock_addr = stem::badaddr; // before vs()!
      PopState( VS_ST_LOCKED );
      lock_rsp.clear();

      rm_lock_safety();

      this->vs_pub_flush();

      stem::Event_base<xmt::uuid_type> flush_ev( VS_FLUSH_VIEW );

      flush_ev.value() = xmt::uid();

      basic_vs::vs( flush_ev );      
    }
  }
}

void basic_vs::vs_flush_lock_view_nak( const stem::EventVoid& ev )
{
  // cerr << __FILE__ << ':' << __LINE__ << endl;
  if ( lock_addr == self_id() ) { // I'm owner of the lock
    lock_addr = stem::badaddr;
    lock_rsp.clear();
    PopState( VS_ST_LOCKED );

    rm_lock_safety();
  }
}

void basic_vs::vs_flush( const xmt::uuid_type& id )
{
  lock_addr = stem::badaddr; // clear lock
  PopState( VS_ST_LOCKED );

  rm_lock_safety();

  // cerr << __FILE__ << ':' << __LINE__ << endl;
  this->vs_pub_flush();
}

// Special case for 're-send rest events', to avoid interference
// with true VS_FLUSH_VIEW---it work with view lock,
// but in this case I want to bypass locking and keep the rest
// functionality, like history writing.

void basic_vs::vs_flush_wr( const xmt::uuid_type& id )
{
  // cerr << __FILE__ << ':' << __LINE__ << endl;
  this->vs_pub_flush();
}


void basic_vs::add_lock_safety()
{
  // belay: avoid infinite lock
  Event_base<CronEntry> cr( EV_EDS_CRON_ADD );
  const stem::EventVoid cr_ev( VS_LOCK_SAFETY );
  cr_ev.dest( self_id() );
  cr_ev.src( self_id() );
  cr_ev.pack( cr.value().ev );

  cr.dest( _cron->self_id() );
  cr.value().start = get_system_time() + vs_lock_timeout;
  cr.value().n = 1;
  cr.value().period = 0;

  Send( cr );
}

void basic_vs::rm_lock_safety()
{
  // remove belay, passed
  Event_base<CronEntry> cr( EV_EDS_CRON_REMOVE );
  const stem::EventVoid cr_ev( VS_LOCK_SAFETY );
  cr_ev.dest( self_id() );
  cr_ev.src( self_id() );
  cr_ev.pack( cr.value().ev );

  cr.dest( _cron->self_id() );

  Send( cr );
}

void basic_vs::vs_lock_safety( const stem::EventVoid& ev )
{
  if ( ev.src() == self_id() ) {
    if ( lock_addr == stem::badaddr ) {
      return; // no lock more
    }

    if ( lock_addr == self_id() ) { // I'm owner of the lock
      vtime& self = vt[self_id()];
      if ( (lock_rsp.size() + 1) < self.vt.size() ) { // not all conforms lock
        if ( (lock_rsp.size() * 2 + 1) >= self.vt.size() ) { // at least half conforms
          // who is in group, but not conform lock?
          for ( vtime::vtime_type::iterator i = self.vt.begin(); i != self.vt.end(); ) {
            if ( (i->first == self_id()) || (lock_rsp.find( i->first ) == lock_rsp.end()) ) {
              ++i;
            } else {
              vt.erase( i->first ); // erase it from group
              self.vt.erase( i++ );
              /* Not required: after next event from node
                 it will be removed
              for ( vtime_matrix_type::iterator j = vt.begin(); j != vt.end(); ++j ) {
                if( j->first != self_id() ) {
                  j->second.erase( i->first );
                }
              }
              */
            }
          }

          // response from all group members available
          ++view;
          if ( group_applicant != stem::badaddr ) {
            vt[self_id()][group_applicant]; // i.e. create entry in vt
            group_applicant = stem::badaddr;
          }
          lock_addr = stem::badaddr; // before vs()!
          PopState( VS_ST_LOCKED );
          lock_rsp.clear();

          rm_lock_safety();

          stem::EventVoid update_view_ev( VS_UPDATE_VIEW );

          basic_vs::vs( update_view_ev );
        } else if ( self.vt.size() == 2 ) { // two nodes in group and one not conform
          // who is in group, but not conform lock?
          for ( vtime::vtime_type::iterator i = self.vt.begin(); i != self.vt.end(); ) {
            if ( i->first == self_id() ) {
              ++i;
            } else {
              vt.erase( i->first ); // erase it from group
              self.vt.erase( i++ );
              /* Not required: after next event from node
                 it will be removed
              for ( vtime_matrix_type::iterator j = vt.begin(); j != vt.end(); ++j ) {
                if( j->first != self_id() ) {
                  j->second.erase( i->first );
                }
              }
              */
              break;
            }
          }

          // response from all group members available
          ++view;
          if ( group_applicant != stem::badaddr ) {
            vt[self_id()][group_applicant]; // i.e. create entry in vt
            group_applicant = stem::badaddr;
          }
          lock_addr = stem::badaddr; // before vs()!
          PopState( VS_ST_LOCKED );
          lock_rsp.clear();

          rm_lock_safety();

          stem::EventVoid update_view_ev( VS_UPDATE_VIEW );

          basic_vs::vs( update_view_ev );
        } else { // problem on this side?
          cerr << __FILE__ << ':' << __LINE__ << ' ' << (lock_rsp.size() + 1) <<  ' ' << self.vt.size() << endl;
        }
      } else {
        // do nothing? shouldn't happen?
        cerr << __FILE__ << ':' << __LINE__ << endl;
      }
      // cerr << __FILE__ << ':' << __LINE__ << ' ' << (lock_rsp.size() + 1) <<  ' ' << self.vt.size() << endl;
      // if ( (lock_rsp.size() + 1) == self.vt.size() ) {
      // }
    } else {
      cerr << __FILE__ << ':' << __LINE__ << ' ' << lock_addr << " (" << self_id() << ')' << endl;
    }
  }
}

const stem::code_type basic_vs::VS_EVENT         = 0x302;
const stem::code_type basic_vs::VS_JOIN_RQ       = 0x304;
const stem::code_type basic_vs::VS_JOIN_RS       = 0x305;
const stem::code_type basic_vs::VS_LOCK_VIEW     = 0x306;
const stem::code_type basic_vs::VS_LOCK_VIEW_ACK = 0x307;
const stem::code_type basic_vs::VS_LOCK_VIEW_NAK = 0x308;
const stem::code_type basic_vs::VS_UPDATE_VIEW   = 0x309;
const stem::code_type basic_vs::VS_FLUSH_LOCK_VIEW = 0x30a;
const stem::code_type basic_vs::VS_FLUSH_LOCK_VIEW_ACK = 0x30b;
const stem::code_type basic_vs::VS_FLUSH_LOCK_VIEW_NAK = 0x30c;
const stem::code_type basic_vs::VS_FLUSH_VIEW    = 0x30d;
const stem::code_type basic_vs::VS_FLUSH_VIEW_JOIN = 0x30e;
const stem::code_type basic_vs::VS_LOCK_SAFETY   = 0x30f;

const stem::state_type basic_vs::VS_ST_LOCKED = 0x10000;

DEFINE_RESPONSE_TABLE( basic_vs )
  EV_Event_base_T_( ST_NULL, VS_EVENT, vs_process, vs_event )
  EV_Event_base_T_( VS_ST_LOCKED, VS_EVENT, vs_process_lk, vs_event )
  EV_Event_base_T_( ST_NULL, VS_JOIN_RQ, vs_join_request, vs_join_rq )
  EV_Event_base_T_( VS_ST_LOCKED, VS_JOIN_RQ, vs_join_request_lk, vs_join_rq )
  EV_Event_base_T_( VS_ST_LOCKED, VS_JOIN_RS, vs_group_points, vs_points )
  EV_Event_base_T_( ST_NULL, VS_LOCK_VIEW, vs_lock_view, void )
  EV_Event_base_T_( VS_ST_LOCKED, VS_LOCK_VIEW, vs_lock_view_lk, void )
  EV_Event_base_T_( VS_ST_LOCKED, VS_LOCK_VIEW_ACK, vs_lock_view_ack, void )
  EV_Event_base_T_( VS_ST_LOCKED, VS_LOCK_VIEW_NAK, vs_lock_view_nak, void )
  EV_VOID( VS_ST_LOCKED, VS_UPDATE_VIEW, vs_view_update )

  EV_Event_base_T_( ST_NULL, VS_FLUSH_LOCK_VIEW, vs_flush_lock_view, void )
  EV_Event_base_T_( VS_ST_LOCKED, VS_FLUSH_LOCK_VIEW, vs_flush_lock_view_lk, void )
  EV_Event_base_T_( VS_ST_LOCKED, VS_FLUSH_LOCK_VIEW_ACK, vs_flush_lock_view_ack, void )
  EV_Event_base_T_( VS_ST_LOCKED, VS_FLUSH_LOCK_VIEW_NAK, vs_flush_lock_view_nak, void )
  EV_T_( VS_ST_LOCKED, VS_FLUSH_VIEW, vs_flush, xmt::uuid_type )
  EV_T_( ST_NULL, VS_FLUSH_VIEW_JOIN, vs_flush_wr, xmt::uuid_type )
  EV_Event_base_T_( VS_ST_LOCKED, VS_LOCK_SAFETY, vs_lock_safety, void )
END_RESPONSE_TABLE

} // namespace janus
