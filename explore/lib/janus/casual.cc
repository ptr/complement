// -*- C++ -*- Time-stamp: <10/03/12 14:52:39 ptr>

/*
 *
 * Copyright (c) 2008-2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <janus/casual.h>

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

static std::tr2::milliseconds vs_lock_timeout( 20000 );

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

basic_vs::basic_vs( const char* info ) :
    EventHandler( info ),
    view( 0 ),
    lock_addr( stem::badaddr )
{
  new( Init_buf ) Init();
}

basic_vs::~basic_vs()
{
  stem::addr_type sid = self_id();

  disable();

  ((Init *)Init_buf)->~Init();

  stem::Event_base<janus::addr_type> sev( VS_LAST_WILL );

  sev.src( stem::badaddr );
  sev.value() = sid;

  for ( vtime::vtime_type::const_iterator i = vt.vt.begin(); i != vt.vt.end(); ++i ) {
    if ( i->first != sid ) {
      // cerr << __FILE__ << ':' << __LINE__ << ' ' << i->first << endl;
      sev.dest( i->first );
      Forward( sev );
      // to first only
      break;
    }
  }

  for ( access_container_type::iterator i = remotes_.begin(); i != remotes_.end(); ++i ) {
    delete *i;
  }
  remotes_.clear();
}

int basic_vs::vs( const stem::Event& inc_ev )
{
  int ret = vs_aux( inc_ev );

  if ( ret == 0 ) {
    // if ( inc_ev.src() == stem::badaddr ) {
    inc_ev.src( self_id() );
    inc_ev.dest( self_id() );
      // }
    inc_ev.setf( stem::__Event_Base::vs );
    basic_vs::sync_call( inc_ev );
  }

  return ret;
}

int basic_vs::vs_aux( const stem::Event& inc_ev )
{
  if ( /* vs_group_size() == 0 || */ /* lock_addr != stem::badaddr || */ isState(VS_ST_LOCKED) ) {
    de.push_back( inc_ev );
    return 1;
  }

  stem::Event_base<vs_event> ev( VS_EVENT );
  ev.value().view = view;
  ev.value().ev = inc_ev;

  ++vt[self_id()];

  const stem::code_type code = inc_ev.code();

  ev.value().ev.setf( stem::__Event_Base::vs );

  for ( vtime::vtime_type::const_iterator i = vt.vt.begin(); i != vt.vt.end(); ++i ) {
    if ( i->first != self_id() ) {
      ev.dest( i->first );
      ev.value().vt = vt;
      Send( ev );
    }
  }

  if ( (code != VS_UPDATE_VIEW) && (code != VS_LOCK_VIEW) && (code != VS_FLUSH_LOCK_VIEW ) ) {
    this->vs_pub_rec( ev.value().ev );
  }

  return 0;
}

void basic_vs::send_to_vsg( const stem::Event& ev ) const // not VS!
{
  stem::Event sev = ev;
  stem::addr_type sid = self_id();

  sev.src( sid );

  for ( vtime::vtime_type::const_iterator i = vt.vt.begin(); i != vt.vt.end(); ++i ) {
    if ( i->first != sid ) {
      sev.dest( i->first );
      Forward( sev );
    }
  }
}

void basic_vs::forward_to_vsg( const stem::Event& ev ) const // not VS!
{
  stem::Event sev = ev;
  stem::addr_type sid = self_id();

  for ( vtime::vtime_type::const_iterator i = vt.vt.begin(); i != vt.vt.end(); ++i ) {
    if ( i->first != sid ) {
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

void basic_vs::vs_copy_tcp_points( const basic_vs& orig )
{
  pair<vs_points::points_type::const_iterator,vs_points::points_type::const_iterator> range = orig.points.equal_range( orig.self_id() );

  for ( ; range.first != range.second; ++range.first ) {
    vs_points::access_t& p = points.insert( make_pair(self_id(), vs_points::access_t()) )->second;
    p = range.first->second;
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

  if ( a == stem::badaddr ) {
    stem::addr_type sid = self_id();
    vt[sid]; // make self-entry not empty (used in vs_group_size)

    vs_pub_join();
    this->vs_pub_view_update();

    while ( !de.empty() ) {
      if ( basic_vs::vs( de.front() ) ) {
        de.pop_back(); // event pushed back in vs() above, remove it
        break;
      }
      de.pop_front();
    }

    return 0;
  }

  return 1;
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

void basic_vs::vs_pub_join()
{
}

basic_vs::size_type basic_vs::vs_group_size() const
{  
  return vt.vt.size();
}

void basic_vs::vs_join_request( const stem::Event_base<vs_join_rq>& ev )
{
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
    if ( vt.vt.size() > 1 ) {
      lock_rsp.clear();
      group_applicant = ev.src();
      group_applicant_ref = ev.value().reference;

      // check: group_applicant re-enter (fail was not detected yet)
      for ( vtime::vtime_type::iterator i = vt.vt.begin(); i != vt.vt.end(); ++i ) {
        if ( i->first == group_applicant ) { // same address
          vt.vt.erase( i );
          break;
        }
      }

      // check net channels (from me)
      for ( access_container_type::iterator i = remotes_.begin(); i != remotes_.end(); ) {
        if ( !(*i)->is_open() || (*i)->bad() ) {
          delete *i;
          remotes_.erase( i++ );
        } else {
          ++i;
        }
      }

      // check vitual time nodes accessibility
      for ( vtime::vtime_type::iterator i = vt.vt.begin(); i != vt.vt.end(); ) {
        if ( !is_avail( i->first ) ) {
          points.erase( i->first );
          vt.vt.erase( i++ );
        } else {
          ++i;
        }
      }

      stem::EventVoid view_lock_ev( VS_LOCK_VIEW );
      basic_vs::vs_aux( view_lock_ev );
      lock_addr = self_id(); // after vs_aux()!
      PushState( VS_ST_LOCKED );

      add_lock_safety(); // belay: avoid infinite lock
    } else { // single in group: lock not required
      this->vs_resend_from( ev.value().reference, ev.src() );

      ++view;
      vt[ev.src()]; // i.e. create entry in vt
      stem::EventVoid update_view_ev( VS_UPDATE_VIEW );
      basic_vs::vs_aux( update_view_ev );

      this->vs_pub_view_update();
    }
  }
}

void basic_vs::vs_join_request_lk( const stem::Event_base<vs_join_rq>& ev )
{
  Event_base<CronEntry> cr( EV_EDS_CRON_ADD );

  ev.pack( cr.value().ev );

  cr.dest( _cron->self_id() );
  cr.value().start = get_system_time() + std::tr2::milliseconds(10);
  cr.value().n = 1;
  cr.value().period = 0;

  Send( cr );
}

void basic_vs::vs_group_points( const stem::Event_base<vs_points>& ev )
{
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
  PushState( VS_ST_LOCKED );
  lock_addr = ev.src();
  stem::EventVoid view_lock_ev( VS_LOCK_VIEW_ACK );
  view_lock_ev.dest( lock_addr );
  Send( view_lock_ev );

  add_lock_safety(); // belay: avoid infinite lock
}

void basic_vs::vs_lock_view_lk( const stem::EventVoid& ev )
{
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
  if ( lock_addr == self_id() ) { // I'm owner of the lock
    lock_rsp.insert( ev.src() );
    /* Below '>=' instead of '==', because of scenario 'one node exit,
       another node added'
     */
    if ( (lock_rsp.size() + 1) >= vt.vt.size() ) {
      for ( vtime::vtime_type::const_iterator i = vt.vt.begin(); i != vt.vt.end(); ++i ) {
        if ( (i->first != self_id()) && (lock_rsp.find( i->first ) == lock_rsp.end()) ) {
          return;
        }
      }
      // response from all group members available
      ++view;
      if ( group_applicant != stem::badaddr ) {
        vt[group_applicant]; // i.e. create entry in vt
        this->vs_resend_from( group_applicant_ref, group_applicant );
        group_applicant = stem::badaddr;
      }
      lock_addr = stem::badaddr; // before vs_aux()!
      PopState( VS_ST_LOCKED );

      lock_rsp.clear();

      rm_lock_safety();

      stem::EventVoid update_view_ev( VS_UPDATE_VIEW );
      basic_vs::vs_aux( update_view_ev );

      this->vs_pub_view_update();

      while ( !de.empty() ) {
        if ( basic_vs::vs( de.front() ) ) {
          de.pop_back(); // event pushed back in vs() above, remove it
          break;
        }
        de.pop_front();
      }
    }
  }
}

void basic_vs::vs_lock_view_nak( const stem::EventVoid& ev )
{
  if ( lock_addr == self_id() ) { // I'm owner of the lock
    lock_addr = stem::badaddr;
    lock_rsp.clear();
    PopState( VS_ST_LOCKED );

    rm_lock_safety();

    while ( !de.empty() ) {
      if ( basic_vs::vs( de.front() ) ) {
        de.pop_back(); // event pushed back in vs() above, remove it
        break;
      }
      de.pop_front();
    }
  }
}

void basic_vs::vs_process( const stem::Event_base<vs_event>& ev )
{
  // check the view version first:
  if ( ev.value().view != view ) {
    if ( ev.value().view > view ) {
      ove.push_back( ev ); // push event into delay queue
    }
    return;
  }

  stem::code_type code = ev.value().ev.code();

  if ( code == VS_UPDATE_VIEW ) {
    ove.push_back( ev ); // push event into delay queue
    return;
  }
  
  vtime tmp = ev.value().vt;

  for ( vtime::vtime_type::const_iterator i = vt.vt.begin(); i != vt.vt.end(); ++i ) {
    if ( i->first == ev.src() ) {
      if ( (i->second + 1) != tmp[ev.src()] ) {
        if ( (i->second + 1) < tmp[ev.src()] ) {
          ove.push_back( ev ); // push event into delay queue
        } else {
          // Ghost event from past: Drop? Error?
        }
        return;
      }
    } else if ( i->second < tmp[i->first] ) {
      ove.push_back( ev ); // push event into delay queue
      return;
    }
  }

  ++vt[ev.src()];
  
  ev.value().ev.src( ev.src() );
  ev.value().ev.dest( ev.dest() );

  if ( (code != VS_LOCK_VIEW) && (code != VS_FLUSH_LOCK_VIEW) ) {
    // Update view not passed into vs_pub_rec,
    // it specific for Virtual Synchrony
    this->vs_pub_rec( ev.value().ev );
  }

  basic_vs::sync_call( ev.value().ev );

  if ( !ove.empty() ) {
    process_delayed();
  }
}

void basic_vs::vs_process_lk( const stem::Event_base<vs_event>& ev )
{
  const stem::code_type code = ev.value().ev.code();
  bool join_final = false;

  if ( ev.value().view != view ) {
    if ( code != VS_UPDATE_VIEW ) {
      ove.push_back( ev ); // push event into delay queue
      return; // ? view changed, but this object unknown yet
    }

    if ( (view != 0) && (lock_addr != ev.src()) ) {
      return; // ? update view: not owner of lock
    }

    if ( (view != 0) && ((view + 1) != ev.value().view) ) {
      ove.push_back( ev ); // push event into delay queue
      return; // ? view changed, but this object unknown yet
    }

    if ( view == 0 ) {
      vt = ev.value().vt.vt; // align time with origin
      --vt[ev.src()]; // will be incremented below
      join_final = true;
    } else { // (view + 1) == ev.value().view
      for ( vtime::vtime_type::iterator i = vt.vt.begin(); i != vt.vt.end(); ) {
        if ( ev.value().vt.vt.find( i->first ) == ev.value().vt.vt.end() ) {
          points.erase( i->first );
          vt.vt.erase( i++ );
          // break;
        } else {
          ++i;
        }
      }
    }
    view = ev.value().view;
  }

  if ( (code != VS_UPDATE_VIEW) && (code != VS_LOCK_VIEW) && (code != VS_FLUSH_VIEW) && (code != VS_FLUSH_LOCK_VIEW) ) {
    ove.push_back( ev ); // push event into delay queue
    return;
  }

  // check virtual time:
  vtime tmp = ev.value().vt;

  for ( vtime::vtime_type::const_iterator i = vt.vt.begin(); i != vt.vt.end(); ++i ) {
    if ( i->first == ev.src() ) {
      if ( (i->second + 1) != tmp[ev.src()] ) {
        if ( (i->second + 1) < tmp[ev.src()] ) {
          ove.push_back( ev ); // push event into delay queue
        } else {
          // Ghost event from past: Drop? Error?
        }
        return;
      }
    } else if ( i->second < tmp[i->first] ) {
      ove.push_back( ev ); // push event into delay queue
      return;
    }
  }

  ++vt[ev.src()];
  
  ev.value().ev.src( ev.src() );
  ev.value().ev.dest( ev.dest() );

  if ( code == VS_UPDATE_VIEW ) {
    /* Specific for update view: vt[self_id()] should
       contain all group members, even if virtual time
       is zero (copy/assign vt don't copy entry with zero vtime!)
    */
    for ( vtime::vtime_type::const_iterator i = ev.value().vt.vt.begin(); i != ev.value().vt.vt.end(); ++i ) {
      vt[i->first];
    }

    lock_addr = stem::badaddr; // clear lock
    PopState( VS_ST_LOCKED );

    rm_lock_safety();

    if ( join_final ) {
      vs_pub_join();
    }
    this->vs_pub_view_update();

    while ( !de.empty() ) {
      if ( basic_vs::vs( de.front() ) ) {
        de.pop_back(); // event pushed back in vs() above, remove it
        break;
      }
      de.pop_front();
    }

    if ( !ove.empty() ) {
      process_delayed();
    }

    return;
  } else if ( code == VS_FLUSH_VIEW ) {
    // flush passed into vs_pub_rec
    this->vs_pub_rec( ev.value().ev );
  }

  basic_vs::sync_call( ev.value().ev );

  while ( !de.empty() ) {
    if ( basic_vs::vs( de.front() ) ) {
      de.pop_back(); // event pushed back in vs() above, remove it
      break;
    }
    de.pop_front();
  }

  if ( !ove.empty() ) {
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

  do {
    delayed_process = false;
    for ( ove_container_type::iterator k = ove.begin(); k != ove.end(); ) {
      tmp = k->value().vt;

      for ( vtime::vtime_type::const_iterator i = vt.vt.begin(); i != vt.vt.end(); ++i ) {
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

      ++vt[k->src()];

      k->value().ev.src( k->src() );
      k->value().ev.dest( k->dest() );

      if ( k->value().ev.code() == VS_UPDATE_VIEW ) {
        /* Specific for update view: vt[self_id()] should
           contain all group members, even if virtual time
           is zero (copy/assign vt don't copy entry with zero vtime!)
        */
        for ( vtime::vtime_type::const_iterator i = k->value().vt.vt.begin(); i != k->value().vt.vt.end(); ++i ) {
          vt[i->first];
        }

        lock_addr = stem::badaddr; // clear lock
        PopState( VS_ST_LOCKED );

        rm_lock_safety();
        this->vs_pub_view_update();
      } else {
        if ( k->value().ev.code() != VS_LOCK_VIEW ) {
          // Update view not passed into vs_pub_rec,
          // it specific for Virtual Synchrony
          this->vs_pub_rec( k->value().ev );
        } else {
          /* Specific for lock view: vt[self_id()] should
             contain all group members, even if virtual time
             is zero (copy/assign vt don't copy entry with zero vtime!)
          */
          for ( vtime::vtime_type::const_iterator i = k->value().vt.vt.begin(); i != k->value().vt.vt.end(); ++i ) {
            vt[i->first];
          }
        }

        basic_vs::sync_call( k->value().ev );
      }

      ove.erase( k++ );
      delayed_process = true;

      try_next:
        ;
    }
  } while ( delayed_process );
}

void basic_vs::replay( const stem::Event& inc_ev )
{
  // here must be: vt[self_id()] <= _vt;
  // another assume: replay called in correct order
  // (vs_pub_rec produce correct order)

  if ( inc_ev.code() != VS_FLUSH_VIEW ) {
    basic_vs::sync_call( inc_ev );
  }
}

void basic_vs::vs_send_flush()
{
  if ( lock_addr == stem::badaddr ) {
    if ( vt.vt.size() > 1 ) {
      lock_rsp.clear();

      stem::EventVoid view_lock_ev( VS_FLUSH_LOCK_VIEW );

      basic_vs::vs_aux( view_lock_ev );
      lock_addr = self_id(); // after vs_aux()!
      PushState( VS_ST_LOCKED );

      add_lock_safety(); // belay: avoid infinite lock
    } else { // single in group: lock not required
      stem::Event_base<xmt::uuid_type> flush_ev( VS_FLUSH_VIEW );
      flush_ev.value() = xmt::uid();
      basic_vs::vs_aux( flush_ev );
      this->vs_pub_flush();
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
  if ( lock_addr == self_id() ) { // I'm owner of the lock
    lock_rsp.insert( ev.src() );
    /* Below '>=' instead of '==', because of scenario 'one node exit,
       another node added'
     */
    if ( (lock_rsp.size() + 1) >= vt.vt.size() ) {
      for ( vtime::vtime_type::const_iterator i = vt.vt.begin(); i != vt.vt.end(); ++i ) {
        if ( (i->first != self_id()) && (lock_rsp.find( i->first ) == lock_rsp.end()) ) {
          return;
        }
      }
      // response from all group members available
      lock_addr = stem::badaddr; // before vs_aux()!
      PopState( VS_ST_LOCKED );
      lock_rsp.clear();
      rm_lock_safety();

      stem::Event_base<xmt::uuid_type> flush_ev( VS_FLUSH_VIEW );
      flush_ev.value() = xmt::uid();
      basic_vs::vs_aux( flush_ev );      
      this->vs_pub_flush();

      while ( !de.empty() ) {
        if ( basic_vs::vs( de.front() ) ) {
          de.pop_back(); // event pushed back in vs() above, remove it
          break;
        }
        de.pop_front();
      }
    }
  }
}

void basic_vs::vs_flush_lock_view_nak( const stem::EventVoid& ev )
{
  if ( lock_addr == self_id() ) { // I'm owner of the lock
    lock_addr = stem::badaddr;
    lock_rsp.clear();
    PopState( VS_ST_LOCKED );

    rm_lock_safety();
    while ( !de.empty() ) {
      if ( basic_vs::vs( de.front() ) ) {
        de.pop_back(); // event pushed back in vs() above, remove it
        break;
      }
      de.pop_front();
    }
  }
}

void basic_vs::vs_flush( const xmt::uuid_type& id )
{
  lock_addr = stem::badaddr; // clear lock
  PopState( VS_ST_LOCKED );

  rm_lock_safety();

  this->vs_pub_flush();

  while ( !de.empty() ) {
    if ( basic_vs::vs( de.front() ) ) {
      de.pop_back(); // event pushed back in vs() above, remove it
      break;
    }
    de.pop_front();
  }
}

// Special case for 're-send rest events', to avoid interference
// with true VS_FLUSH_VIEW---it work with view lock,
// but in this case I want to bypass locking and keep the rest
// functionality, like history writing.

void basic_vs::vs_flush_wr( const xmt::uuid_type& id )
{
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
  if ( ev.src() != self_id() ) {
    return;
  }

  if ( lock_addr == stem::badaddr ) {
    return; // no lock more
  }

  // Check channels
  for ( access_container_type::iterator i = remotes_.begin(); i != remotes_.end(); ) {
    if ( !(*i)->is_open() || (*i)->bad() ) {
      delete *i;
      remotes_.erase( i++ );
    } else {
      ++i;
    }
  }

  if ( lock_addr != self_id() ) {
    cerr << __FILE__ << ':' << __LINE__ << ' ' << lock_addr << " (" << self_id() << ')' << ' ' << is_avail(lock_addr) << endl;
    // if ( is_avail(lock_addr) ) {
    return;
    // }
  }

  // I'm owner of the lock
  // if ( (lock_rsp.size() + 1) < vt.vt.size() ) { // not all conforms lock
  // at least half conforms, or group small and unstable;
  // in case of 'too small' group 'no conformation' may happens!
  if ( ((lock_rsp.size() * 2 + 1) >= vt.vt.size()) || (vt.vt.size() <= 2) ) {
    // who is in group, but not conform lock?
    for ( vtime::vtime_type::iterator i = vt.vt.begin(); i != vt.vt.end(); ) {
      if ( (i->first == self_id()) || (lock_rsp.find( i->first ) != lock_rsp.end()) ) {
        ++i;
      } else {
        points.erase( i->first );
        vt.vt.erase( i++ );
      }
    }

    // response from all group members available
    ++view;
    if ( group_applicant != stem::badaddr ) {
      vt[group_applicant]; // i.e. create entry in vt
      group_applicant = stem::badaddr;
    }
    lock_addr = stem::badaddr; // before vs_aux()!
    PopState( VS_ST_LOCKED );
    lock_rsp.clear();

    rm_lock_safety();

    stem::EventVoid update_view_ev( VS_UPDATE_VIEW );

    basic_vs::vs_aux( update_view_ev );

    while ( !de.empty() ) {
      if ( basic_vs::vs( de.front() ) ) {
        de.pop_back(); // event pushed back in vs() above, remove it
        break;
      }
      de.pop_front();
    }
  } else { // problem on this side?
    cerr << __FILE__ << ':' << __LINE__ << ' ' << (lock_rsp.size() + 1) <<  ' ' << vt.vt.size() << endl;
  }
  // } else {
  // do nothing? shouldn't happen?
  //   cerr << __FILE__ << ':' << __LINE__ << endl;
  // }
  // cerr << __FILE__ << ':' << __LINE__ << ' ' << (lock_rsp.size() + 1) <<  ' ' << vt.vt.size() << endl;
  // if ( (lock_rsp.size() + 1) == vt.vt.size() ) {
  // }
}

void basic_vs::process_last_will( const stem::Event_base<janus::addr_type>& ev )
{
  // Don't check ev.src() here: it may be badaddr
  if ( ev.value() == self_id() ) { // illegal usage
    return; 
  }

  if ( lock_addr == stem::badaddr ) {
    points.erase( ev.value() );
    vt.vt.erase( ev.value() );

    if ( vt.vt.size() > 1 ) {
      lock_rsp.clear();

      group_applicant = stem::badaddr;

      stem::EventVoid view_lock_ev( VS_LOCK_VIEW );

      basic_vs::vs_aux( view_lock_ev );
      lock_addr = self_id(); // after vs_aux()!
      PushState( VS_ST_LOCKED );

      add_lock_safety(); // belay: avoid infinite lock
    } else { // single in group
      ++view;
      stem::EventVoid update_view_ev( VS_UPDATE_VIEW );

      basic_vs::vs_aux( update_view_ev );

      this->vs_pub_view_update();
    }
  } else {
    cerr << __FILE__ << ':' << __LINE__ << ' ' << self_id() << endl;
  }
}

void basic_vs::process_last_will_lk( const stem::Event_base<janus::addr_type>& ev )
{
}

void basic_vs::check_remotes()
{
  for ( access_container_type::iterator i = remotes_.begin(); i != remotes_.end(); ) {
    if ( !(*i)->is_open() || (*i)->bad() ) {
      delete *i;
      remotes_.erase( i++ );
    } else {
      ++i;
    }
  }

  bool drop = false;
  for ( vtime::vtime_type::iterator i = vt.vt.begin(); i != vt.vt.end(); ) {
    if ( !is_avail( i->first ) ) {
      points.erase( i->first );
      vt.vt.erase( i++ );
      drop = true;
    } else {
      ++i;
    }
  }

  if ( drop ) {
    if ( !isState(VS_ST_LOCKED) ) {
      if ( vt.vt.size() > 1 ) {
        lock_rsp.clear();

        group_applicant = stem::badaddr;
        stem::EventVoid view_lock_ev( VS_LOCK_VIEW );

        basic_vs::vs_aux( view_lock_ev );
        lock_addr = self_id(); // after vs_aux()!
        PushState( VS_ST_LOCKED );

        add_lock_safety(); // belay: avoid infinite lock
      } else { // single in group: lock not required
        ++view;
      }
    } else {
      const stem::EventVoid cr_ev( VS_LOCK_SAFETY );
      cr_ev.dest( self_id() );
      Send( cr_ev );
    }
  }
}

void basic_vs::access_points_refresh()
{
  this->pub_access_point(); // user-defined

  stem::addr_type sid = self_id();

  pair<vs_points::points_type::const_iterator,vs_points::points_type::const_iterator> range = points.equal_range( sid );

  stem::Event_base<janus::detail::access_points> my( VS_ACCESS_POINT_PRI );

  for ( ; range.first != range.second; ++range.first ) {
    my.value().points.insert( *range.first );
  }

  send_to_vsg( my );
}

void basic_vs::pub_access_point()
{
  // fill own access point
}

void basic_vs::access_points_refresh_pri( const stem::Event_base<janus::detail::access_points>& ev )
{
  if ( ev.src() == self_id() ) {
    return;
  }

  // points.clear(); ?

  vtime::vtime_type::const_iterator i = vt.vt.find( ev.src() );

  if ( i == vt.vt.end() ) {
    return;
  }

  this->pub_access_point(); // user-defined

  pair<vs_points::points_type::const_iterator,vs_points::points_type::const_iterator> range = points.equal_range( self_id() );

  stem::Event_base<janus::detail::access_points> my( VS_ACCESS_POINT_SEC );

  for ( ; range.first != range.second; ++range.first ) {
    my.value().points.insert( *range.first );
  }

  send_to_vsg( my );

  range = ev.value().points.equal_range( ev.src() );

  for ( ; range.first != range.second; ++range.first ) {
    points.insert( *range.first );
  }
}

void basic_vs::access_points_refresh_sec( const stem::Event_base<janus::detail::access_points>& ev )
{
  if ( ev.src() == self_id() ) {
    return;
  }

  vtime::vtime_type::const_iterator i = vt.vt.find( ev.src() );

  if ( i == vt.vt.end() ) {
    return;
  }

  pair<vs_points::points_type::const_iterator,vs_points::points_type::const_iterator> range = ev.value().points.equal_range( ev.src() );
  for ( ; range.first != range.second; ++range.first ) {
    vs_points::access_t& p = points.insert( make_pair(ev.src(), vs_points::access_t()) )->second;
    p = range.first->second;
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
const stem::code_type basic_vs::VS_LAST_WILL     = 0x310;
const stem::code_type basic_vs::VS_ACCESS_POINT_PRI = 0x311;
const stem::code_type basic_vs::VS_ACCESS_POINT_SEC = 0x312;

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

  EV_Event_base_T_( ST_NULL, VS_FLUSH_LOCK_VIEW, vs_flush_lock_view, void )
  EV_Event_base_T_( VS_ST_LOCKED, VS_FLUSH_LOCK_VIEW, vs_flush_lock_view_lk, void )
  EV_Event_base_T_( VS_ST_LOCKED, VS_FLUSH_LOCK_VIEW_ACK, vs_flush_lock_view_ack, void )
  EV_Event_base_T_( VS_ST_LOCKED, VS_FLUSH_LOCK_VIEW_NAK, vs_flush_lock_view_nak, void )
  EV_T_( VS_ST_LOCKED, VS_FLUSH_VIEW, vs_flush, xmt::uuid_type )
  EV_T_( ST_NULL, VS_FLUSH_VIEW_JOIN, vs_flush_wr, xmt::uuid_type )
  EV_Event_base_T_( VS_ST_LOCKED, VS_LOCK_SAFETY, vs_lock_safety, void )

  EV_Event_base_T_( ST_NULL, VS_LAST_WILL, process_last_will, janus::addr_type )
  EV_Event_base_T_( VS_ST_LOCKED, VS_LAST_WILL, process_last_will_lk, janus::addr_type )
  EV_Event_base_T_( ST_NULL, VS_ACCESS_POINT_PRI, access_points_refresh_pri, janus::detail::access_points )
  EV_Event_base_T_( ST_NULL, VS_ACCESS_POINT_SEC, access_points_refresh_sec, janus::detail::access_points )
END_RESPONSE_TABLE

} // namespace janus
