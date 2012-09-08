// -*- C++ -*- Time-stamp: <2012-02-17 15:32:45 ptr>

/*
 *
 * Copyright (c) 2008-2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <janus/causal.h>

#include <stdint.h>

#include <sys/socket.h>
#include <sockios/sockstream>
#include <stem/NetTransport.h>
#include <stem/Cron.h>
#include <stem/EDSEv.h>

#include <mt/mutex>
#include <sockios/syslog.h>

namespace janus {

using namespace std;
using namespace xmt;
using namespace stem;
using namespace std::tr2;

#define VS_EVENT            0x300
#define VS_JOIN_RQ          0x301
#define VS_JOIN_RS          0x302
#define VS_LOCK_VIEW        0x303
#define VS_LOCK_VIEW_ACK    0x304
#define VS_LOCK_VIEW_NAK    0x305
#define VS_UPDATE_VIEW      0x306
#define VS_FLUSH_RQ         0x307
#define VS_LOCK_SAFETY      0x308 // from cron, timeout
#define VS_ACCESS_POINT_PRI 0x309
#define VS_ACCESS_POINT_SEC 0x30a
#define VS_ACCESS_POINT     0x30b

const janus::addr_type nil_addr( xmt::nil_uuid, xmt::nil_uuid );
const gid_type& nil_gid = xmt::nil_uuid;

static std::tr2::milliseconds vs_lock_timeout( 20000 );

static char* Init_buf[128];
Cron* basic_vs::_cron = 0;

static int *_rcount = 0;

const int /* basic_vs:: */ max_vs_lock_safety_sequental_attempts = 1 /* 00 */;

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
    lock_addr( stem::extbadaddr ),
    group_applicant( stem::extbadaddr ),
    sid( self_id() ),
    self_events( 0 ),
    vs_lock_safety_sequental_attempts( 0 )
{
  new( Init_buf ) Init();
}

basic_vs::basic_vs( const char* info ) :
    EventHandler( info ),
    view( 0 ),
    lock_addr( stem::extbadaddr ),
    group_applicant( stem::extbadaddr ),
    sid( self_id() ),
    self_events( 0 ),
    vs_lock_safety_sequental_attempts( 0 )
{
  new( Init_buf ) Init();
}

basic_vs::~basic_vs()
{
  cerr << HERE << ' ' << vs_group_size() << endl;

  for ( access_container_type::iterator i = remotes_.begin(); i != remotes_.end(); ++i ) {
    delete *i;
  }
  remotes_.clear();

  ((Init *)Init_buf)->~Init();
}

std::tr2::milliseconds basic_vs::vs_pub_lock_timeout() const
{
  return vs_lock_timeout;
}

int basic_vs::vs( const stem::Event& inc_ev )
{
  lock_guard<recursive_mutex> hlk( _theHistory_lock );

  if ( isState(VS_ST_LOCKED) ) {
    // misc::use_syslog<LOG_DEBUG>() << "de.push_back()" << ':' << HERE << ':' << sid << ':' << inc_ev.code() << endl;
    de.push_back( inc_ev );
    return 1;
  }

  stem::Event_base<vs_event> ev( VS_EVENT );

  ev.src( make_pair( stem::EventHandler::domain(), sid ) );
  
  {
    lock_guard<recursive_mutex> lkv( _lock_vt );

    ++vt[ev.src()]; // sid
    ++self_events;
    ev.value().view = view;
    ev.value().ev = inc_ev;
    ev.value().vt = vt;
    ev.value().ev.setf( stem::__Event_Base::vs );
    // ev.src( sid );

    for ( vtime::vtime_type::const_iterator i = vt.vt.begin(); i != vt.vt.end(); ++i ) {
      ev.dest( i->first );
      Forward( ev );
    }
  }

  return 0;
}

int basic_vs::vs_locked( const stem::Event& inc_ev )
{
  lock_guard<recursive_mutex> hlk( _theHistory_lock );

  stem::Event_base<vs_event> ev( VS_EVENT );
  ev.src( make_pair( stem::EventHandler::domain(), sid ) );

  {
    lock_guard<recursive_mutex> lk( _lock_vt );

    ++vt[ev.src()]; // sid
    ++self_events;
    ev.value().view = view;
    ev.value().ev = inc_ev;
    ev.value().vt = vt;
    ev.value().ev.setf( stem::__Event_Base::vs );

    for ( vtime::vtime_type::const_iterator i = vt.vt.begin(); i != vt.vt.end(); ++i ) {
      ev.dest( i->first );
      Forward( ev );
    }
  }

  return 0;
}

void basic_vs::vs_process( const stem::Event_base<vs_event>& ev )
{
  // check the view version first:
  if ( view != 0 && ev.value().view != view ) {
    if ( ev.value().view > view ) {
      // misc::use_syslog<LOG_DEBUG>() << "ove.push_back" << ':' << HERE << ':' << sid << ':' << ev.value().ev.code() << endl;
      ove.push_back( ev ); // push event into delay queue
    } else {
      misc::use_syslog<LOG_DEBUG>() << HERE << ':' << sid << ":unexpected" << endl;
    }

    return;
  }

  stem::code_type code = ev.value().ev.code();

  {
    // check that first message is from join responsible member and is VS_UPDATE_VIEW
    lock_guard<recursive_mutex> lk( _lock_vt );

    if ( vt.vt.empty() ) {
      if ( ev.src() != lock_addr || code != VS_UPDATE_VIEW ) {
        // misc::use_syslog<LOG_DEBUG>() << "ove.push_back" << ':' << __FILE__ << ':' << __LINE__ << ':' << sid << ':' << ev.value().ev.code() << endl;
        ove.push_back( ev );
        return;
      }
    }

    if ( self_events && ev.src().second != sid ) {
      // misc::use_syslog<LOG_DEBUG>() << "ove.push_back" << ':' << __FILE__ << ':' << __LINE__ << ':' << sid << ':' << ev.value().ev.code() << endl;
      ove.push_back( ev );
      return;
    }

    vtime tmp = ev.value().vt;

    if ( !check_remotes() ) {
      vs_send_flush(); 
    }

    for ( vtime::vtime_type::const_iterator i = vt.vt.begin(); i != vt.vt.end(); ++i ) {
      if ( (i->first == ev.src()) && (i->first.second != sid) ) {
        if ( (i->second + 1) != tmp[ev.src()] ) {
          if ( (i->second + 1) < tmp[ev.src()] ) {
            // misc::use_syslog<LOG_DEBUG>() << "ove.push_back" << ':' << HERE << ':' << sid << ':' << ev.src() << ':' << ev.value().ev.code() << endl;
            ove.push_back( ev ); // push event into delay queue
          } else {
            misc::use_syslog<LOG_DEBUG>() << HERE << ':' << sid << ':' << ev.value().ev.code() << ":unexpected" << endl;
            // Ghost event from past: Drop? Error?
          }
          return;
        }
      } else if ( i->second < tmp[i->first] ) {
        // misc::use_syslog<LOG_DEBUG>() << "ove.push_back" << ':' << HERE << ':' << sid << ':' << ev.src() << ':' << ev.value().ev.code() << endl;
        ove.push_back( ev ); // push event into delay queue
        return;
      }
    }

    if ( ev.src().second != sid ) {
      ++vt[ev.src()];
    } else {
      --self_events;
    }

    ev.value().ev.src( ev.src() );
    ev.value().ev.dest( make_pair(stem::EventHandler::domain(), sid) );

    if ( (code != VS_UPDATE_VIEW) && (code != VS_LOCK_VIEW) ) {
      this->vs_pub_rec( ev.value().ev );
    }
  }

  this->Dispatch( ev.value().ev );

  // required even for event in right order
  process_out_of_order();
}


void basic_vs::send_to_vsg( const stem::Event& ev ) const // not VS!
{
  stem::Event sev = ev;
  sev.src( make_pair(stem::EventHandler::domain(), sid) );
  forward_to_vsg( sev );
}

void basic_vs::forward_to_vsg( const stem::Event& ev ) const // not VS!
{
  stem::Event sev = ev;

  lock_guard<recursive_mutex> lk( _lock_vt );

  for ( vtime::vtime_type::const_iterator i = vt.vt.begin(); i != vt.vt.end(); ++i ) {
    if ( i->first.second != sid ) {
      sev.dest( i->first );
      Forward( sev );
    }
  }
}

// addr in network byte order, port in native byte order
void basic_vs::vs_tcp_point( uint32_t addr, int port )
{
  string d;
  d.resize( /* sizeof(uint32_t) + sizeof(uint16_t) */ 6 );
  memcpy( (void *)d.data(), (const void *)&addr, 4 );
  uint16_t prt = stem::to_net( static_cast<uint32_t>(port) );
  memcpy( (void *)(d.data() + 4), (const void *)&prt, 2 );

  pair<vs_points::points_type::const_iterator,vs_points::points_type::const_iterator> range = points.equal_range( make_pair(stem::EventHandler::domain(), sid) );
  for ( ; range.first != range.second; ++range.first ) {
    // hostid is equal: select by self_id()
    if ( (range.first->second.family == AF_INET) &&
         (range.first->second.type == std::sock_base::sock_stream) &&
         (range.first->second.data == d) ) {
      return; // already in points
    }
  }

  vs_points::access_t& p = points.insert( make_pair(make_pair(stem::EventHandler::domain(), sid), vs_points::access_t()) )->second;

  p.hostid = xmt::hostid();
  p.family = AF_INET;
  p.type = std::sock_base::sock_stream;
#if 0
  p.data.resize( /* sizeof(uint32_t) + sizeof(uint16_t) */ 6 );
  memcpy( (void *)p.data.data(), (const void *)&addr, 4 );
  uint16_t prt = stem::to_net( static_cast<uint32_t>(port) );
  memcpy( (void *)(p.data.data() + 4), (const void *)&prt, 2 );
#else
  swap( p.data, d );
#endif
}

void basic_vs::vs_tcp_point( const sockaddr_in& a )
{
  if ( a.sin_family == PF_INET ) {
    string d;
    d.resize( /* sizeof(uint32_t) + sizeof(uint16_t) */ 6 );
    memcpy( (void *)d.data(), (const void *)&a.sin_addr, 4 );
    memcpy( (void *)(d.data() + 4), (const void *)&a.sin_port, 2 );


    pair<vs_points::points_type::const_iterator,vs_points::points_type::const_iterator> range = points.equal_range( make_pair(stem::EventHandler::domain(), sid) );
    for ( ; range.first != range.second; ++range.first ) {
      // hostid is equal: select by self_id()
      if ( (range.first->second.family == AF_INET) &&
           (range.first->second.type == std::sock_base::sock_stream) &&
           (range.first->second.data == d) ) {
        return; // already in points
      }
    }

    vs_points::access_t& p = points.insert( make_pair(make_pair(stem::EventHandler::domain(), sid), vs_points::access_t()) )->second;

    p.hostid = xmt::hostid();
    p.family = AF_INET;
    p.type = std::sock_base::sock_stream;
#if 0
    p.data.resize( /* sizeof(uint32_t) + sizeof(uint16_t) */ 6 );
    memcpy( (void *)p.data.data(), (const void *)&a.sin_addr, 4 );
    memcpy( (void *)(p.data.data() + 4), (const void *)&a.sin_port, 2 );
#else
    swap( p.data, d );
#endif
  }
}

void basic_vs::vs_copy_tcp_points( const basic_vs& orig )
{
  pair<vs_points::points_type::const_iterator,vs_points::points_type::const_iterator> range = orig.points.equal_range( make_pair(stem::EventHandler::domain(), orig.self_id()) );

  for ( ; range.first != range.second; ++range.first ) {
    vs_points::access_t& p = points.insert( make_pair(make_pair(stem::EventHandler::domain(), sid), vs_points::access_t()) )->second;
    p = range.first->second;
  }
}

int basic_vs::vs_join( const stem::ext_addr_type& a )
{
  if ( a.second == self_id() ) {
    return 1; // join to self prohibited
  }

  if ( a.second == stem::badaddr ) {
    this->vs_pub_recover( true );
    _lock_vt.lock();
    vt[make_pair(stem::EventHandler::domain(), sid)]; // make self-entry not empty (used in vs_group_size)
    _lock_vt.unlock();

    vs_pub_join();
    this->vs_pub_view_update();

    return 0;
  }

  // if ( is_avail( a.second ) ) {
    cerr << HERE << endl;
    xmt::uuid_type ref = this->vs_pub_recover( false );

    PushState( VS_ST_LOCKED );
    lock_addr = a;

    stem::Event_base<vs_join_rq> ev( VS_JOIN_RQ );

    ev.dest( a );
    ev.value().points = points;
    ev.value().reference = ref;

    Send( ev );

    add_lock_safety();  // belay: avoid infinite lock

    return 0;
  // }

  // return 1;
}

int basic_vs::vs_join( const stem::ext_addr_type& a, const char* host, int port )
{
#if 0 // review
  if ( /* !is_avail( a ) */ true ) {
    if ( a.second == stem::badaddr ) {
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
    remotes_.back()->add_remote_route( sid );
  }
#endif // review

  return vs_join( a );
}

int basic_vs::vs_join( const stem::ext_addr_type& a, const sockaddr_in& srv )
{
#if 0 // review
  if ( /* !is_avail( a ) */ true ) {
    if ( a.second == stem::badaddr ) {
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
    remotes_.back()->add_remote_route( sid );
  }
#endif // review

  return vs_join( a );
}

int basic_vs::vs_join( const char* host, int port )
{
  int ret = 6;
  list<sockaddr> haddrs;
  gethostaddr2( host, back_inserter(haddrs) );
  for ( list<sockaddr>::const_iterator i = haddrs.begin(); i != haddrs.end(); ++i ) {
    switch ( i->sa_family ) {
      case PF_INET:
        ((sockaddr_in *)&*i)->sin_port = htons( static_cast<int16_t>(port) );
        ret = vs_join( *((sockaddr_in *)&*i) );
        if ( ret == 0 ) {
          return 0;
        }
        break;
      case PF_INET6:
        break;
    }
  }

  return ret;
}

int basic_vs::vs_join( const sockaddr_in& a )
{
  // Checks, to avoid joining to self:
  string d;
  d.resize( /* sizeof(uint32_t) + sizeof(uint16_t) */ 6 );
  memcpy( (void *)d.data(), (const void *)&a.sin_addr, 4 );
  memcpy( (void *)(d.data() + 4), (const void *)&a.sin_port, 2 );

  bool local = false;

  pair<vs_points::points_type::const_iterator,vs_points::points_type::const_iterator> range = points.equal_range( make_pair(stem::EventHandler::domain(), sid) );
  for ( ; range.first != range.second; ++range.first ) {
    // hostid is equal: select by self_id()
    if ( (range.first->second.family == AF_INET) &&
         (range.first->second.type == std::sock_base::sock_stream) &&
         (range.first->second.data == d) ) {
#if 0 // review
      if ( get_default() == sid ) {
        return 5; // attempt to join to self detected
      }
#endif // review
      local = true; // not self, but object is local; skip network later
    }
  }

#if 0 // review
  if ( local ) {
    return vs_join( get_default() ); // local object, skip network
  }

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
  remotes_.back()->add_remote_route( sid );
#else
  stem::ext_addr_type trial_node;
#endif // review

  return vs_join( trial_node );
}

void basic_vs::vs_pub_join()
{
}

basic_vs::size_type basic_vs::vs_group_size() const
{
  lock_guard<recursive_mutex> lk( _lock_vt );
  return vt.vt.size();
}


void basic_vs::vs_join_request_work( const stem::Event_base<vs_join_rq>& ev )
{
  cerr << HERE << endl;
  check_remotes();

  stem::Event_base<vs_points> rsp( VS_JOIN_RS );

  rsp.value().points = points;
  rsp.dest( ev.src() );

  rsp.src( make_pair( stem::EventHandler::domain(), sid ) );
  Forward( rsp );

  {
    stem::Event_base< vs_points > pev( VS_ACCESS_POINT );
    pev.value().points = ev.value().points;
    vs_access_point( pev );
    send_to_vsg( pev );
  }

  group_applicant = ev.src();
  group_applicant_ref = ev.value().reference;

  _lock_vt.lock();

  // check: group_applicant re-enter (fail was not detected yet)
  for ( vtime::vtime_type::iterator i = vt.vt.begin(); i != vt.vt.end(); ++i ) {
    if ( i->first == group_applicant ) { // same address
      vt.vt.erase( i ); // ok, thanks to break below
      break;
    }
  }

  _lock_vt.unlock();

  // misc::use_syslog<LOG_DEBUG>() << "vs_join_request_work:" << sid << ':' << group_applicant << endl;

  lock_rsp.clear();
  stem::EventVoid view_lock_ev( VS_LOCK_VIEW );
  basic_vs::vs( view_lock_ev );

  view_lock_ev.dest( group_applicant );
  view_lock_ev.src( make_pair( stem::EventHandler::domain(), sid ) );
  Forward( view_lock_ev );
}

void basic_vs::vs_join_request( const stem::Event_base<vs_join_rq>& ev )
{
  fq.push_back( stem::detail::convert<stem::Event_base<vs_join_rq>,stem::Event>()(ev) );
  
  if ( fq.size() == 1 ) {
    vs_join_request_work( ev );
  }
}

void basic_vs::vs_join_request_lk( const stem::Event_base<vs_join_rq>& ev )
{
  fq.push_back( stem::detail::convert<stem::Event_base<vs_join_rq>,stem::Event>()(ev) );
}

void basic_vs::vs_send_flush()
{
  cerr << HERE << ' ' << self_id() << endl;
  Event_base< xmt::uuid_type > ev( VS_FLUSH_RQ );
  ev.value() = xmt::uid();
  ev.dest( make_pair( stem::EventHandler::domain(), sid ) );
  ev.src( make_pair( stem::EventHandler::domain(), sid ) );
  Forward( ev );
}

void basic_vs::vs_flush_request_work( const stem::Event_base< xmt::uuid_type >& ev )
{
  cerr << HERE << ' ' << self_id() << endl;
  // misc::use_syslog<LOG_DEBUG>() << "vs_flush_request_work:" << sid << endl;
  check_remotes();
  
  lock_rsp.clear();
  stem::EventVoid view_lock_ev( VS_LOCK_VIEW );
  basic_vs::vs( view_lock_ev );
}

void basic_vs::vs_flush_request( const stem::Event_base< xmt::uuid_type >& ev )
{
  cerr << HERE << endl;
  fq.push_back( stem::detail::convert<stem::Event_base<xmt::uuid_type>,stem::Event>()( ev ) );

  if ( fq.size() == 1 ) {
    vs_flush_request_work( ev );
  }
}

void basic_vs::vs_flush_request_lk( const stem::Event_base< xmt::uuid_type >& ev )
{
  cerr << HERE << endl;
  fq.push_back( stem::detail::convert<stem::Event_base<xmt::uuid_type>,stem::Event>()( ev ) );
}

void basic_vs::repeat_request( const stem::Event& ev )
{
  switch ( ev.code() ) {
    case VS_JOIN_RQ:
      vs_join_request_work( stem::detail::convert<stem::Event, stem::Event_base<vs_join_rq> >()(ev) );
      break;
    case VS_FLUSH_RQ:
      vs_flush_request_work( stem::detail::convert<stem::Event, stem::Event_base<xmt::uuid_type> >()(ev) );
      break;
    default:
      // shouldn't happens
      break;
  }
}

void basic_vs::vs_lock_view( const stem::EventVoid& ev )
{
  cerr << HERE << ' ' << self_id() << ' ' << ev.src().second << endl;
  PushState( VS_ST_LOCKED );
  lock_addr = ev.src();
  // misc::use_syslog<LOG_DEBUG>() << "vs_lock_view:" << sid << ':' << lock_addr << endl;
  stem::EventVoid view_lock_ev( VS_LOCK_VIEW_ACK );
  view_lock_ev.dest( lock_addr );

  view_lock_ev.src( make_pair( stem::EventHandler::domain(), sid ) );
  Forward( view_lock_ev );

  add_lock_safety(); // belay: avoid infinite lock
}

void basic_vs::vs_lock_view_lk( const stem::EventVoid& ev )
{
  // misc::use_syslog<LOG_DEBUG>() << "vs_lock_view_lk1:" << sid << ':' << lock_addr << endl;
  if ( ev.src() <= lock_addr ) {
    lock_addr = ev.src();
    // misc::use_syslog<LOG_DEBUG>() << "vs_lock_view_lk2:" << sid << ':' << lock_addr << endl;
    stem::EventVoid view_lock_ev( VS_LOCK_VIEW_ACK );
    view_lock_ev.dest( lock_addr );
    view_lock_ev.src( make_pair( stem::EventHandler::domain(), sid ) );
    Forward( view_lock_ev );
  }
}

void basic_vs::check_lock_rsp()
{
  if ( !fq.empty() ) {
    unique_lock<recursive_mutex> lk( _lock_vt );
    // misc::use_syslog<LOG_DEBUG>() << "check_lock_rsp:" << sid << ':' << lock_rsp.size() << ':' << vt.vt.size() << ':' << (fq.front().code() == VS_JOIN_RQ) << endl;
    if ( lock_rsp.size() >= (vt.vt.size() + (fq.front().code() == VS_JOIN_RQ)) ) {
      for ( vtime::vtime_type::const_iterator i = vt.vt.begin(); i != vt.vt.end(); ++i ) {
        if ( (lock_rsp.find( i->first ) == lock_rsp.end()) ) {
          // misc::use_syslog<LOG_DEBUG>() << HERE << " unexpected " << sid << ' ' << i->first << endl;
          return;
        }
      }
      // response from all group members available
      if ( group_applicant != stem::extbadaddr ) {
        vt[group_applicant]; // i.e. create entry in vt
        this->vs_resend_from( group_applicant_ref, group_applicant );
        group_applicant = stem::extbadaddr;
        group_applicant_ref = xmt::nil_uuid;
      }
        
      stem::Event_base< pair< stem::ext_addr_type, string > > iev( fq.front().code() );
      iev.value().first = fq.front().src();
      if ( fq.front().code() == VS_FLUSH_RQ ) {
        iev.value().second = fq.front().value();
      }

      stem::Event_base<vs_event> update_view_ev( VS_UPDATE_VIEW );
      update_view_ev.value().view = view;
      update_view_ev.value().vt = vt;
      update_view_ev.value().ev = stem::detail::convert<stem::Event_base< pair< stem::ext_addr_type, string > >, stem::Event>()(iev);

      lk.unlock();
      vs_locked( update_view_ev );
    }
  } else {
    misc::use_syslog<LOG_DEBUG>() << HERE << ":unexpected" << endl;
  }
}

void basic_vs::vs_lock_view_ack( const stem::EventVoid& ev )
{
  lock_rsp.insert( ev.src() );
  check_lock_rsp();
}

void basic_vs::vs_update_view( const Event_base<vs_event>& ev )
{
  lock_guard<recursive_mutex> hlk( _theHistory_lock );

  cerr << HERE << endl;

  lock_addr = stem::extbadaddr;
  vs_lock_safety_sequental_attempts = 0;
  RemoveState( VS_ST_LOCKED );

  {
    // belay: avoid infinite lock
    Event_base<CronEntry> cr( EV_EDS_CRON_REMOVE );
    const stem::EventVoid cr_ev( VS_LOCK_SAFETY );
    cr_ev.dest( make_pair( stem::EventHandler::domain(), sid ) );
    cr_ev.src( make_pair( stem::EventHandler::domain(), sid ) );
    cr_ev.pack( cr.value().ev );

    cr.dest( make_pair( stem::EventHandler::domain(), _cron->self_id() ) );
    // cr.value().start = get_system_time() + vs_pub_lock_timeout();
    // cr.value().n = 1;
    // cr.value().period = 0;

    cr.src( make_pair( stem::EventHandler::domain(), sid ) );
    Forward( cr );
  }

  stem::code_type code = ev.value().ev.code();
  stem::Event_base< pair< stem::ext_addr_type, string > > origin_event = stem::detail::convert< stem::Event, stem::Event_base< pair< stem::ext_addr_type, string > > >()(ev.value().ev);
  stem::ext_addr_type origin = origin_event.value().first;

  if ( code == VS_JOIN_RQ ) {
    _lock_vt.lock();
    view = ev.value().view + 1;
    vt = ev.value().vt;

    _lock_vt.unlock();

    if ( origin.second == sid ) {
      vs_pub_join();
    } 
    // misc::use_syslog<LOG_INFO,LOG_USER>() << "vs_pub_update_view:" << sid << endl;
    this->vs_pub_view_update();
  } else if ( code == VS_FLUSH_RQ ) {
    Event tmp_ev( code );
    tmp_ev.value() = origin_event.value().second;
    vs_pub_rec( tmp_ev );
    this->vs_pub_flush();
  }

  if ( !fq.empty() && (ev.src().second == sid) && (fq.front().dest().second == sid) ) {
    fq.pop_front();
  }

  process_delayed();
}

void basic_vs::vs_group_points( const stem::Event_base<vs_points>& ev )
{
#if 0 // review
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
              remotes_.back()->add_remote_route( sid );
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
            remotes_.back()->add_remote_route( sid );
          }
        }
      }
    }
  }
#endif // review
}

void basic_vs::process_delayed()
{
  while ( !de.empty() ) {
    if ( basic_vs::vs( de.front() ) ) {
      de.pop_back(); // event pushed back in vs() above, remove it
      break;
    }
    // misc::use_syslog<LOG_INFO,LOG_USER>() << "de.pop_front()" << ':' << HERE << ':' << sid << ':' << de.front().code() << endl;
    de.pop_front();
  }

  if ( !fq.empty() ) {
    repeat_request( fq.front() );
  }
}

void basic_vs::process_out_of_order()
{
  /*
    for each event in delay_queue try to process it;
    repeat procedure if any event from delay_queue was processed.
   */

  bool delayed_process;
  vtime tmp;
  Event ev;

  do {
    delayed_process = false;
    for ( ove_container_type::iterator k = ove.begin(); k != ove.end(); ) {
      tmp = k->value().vt;

      _lock_vt.lock();

      if ( self_events && k->src().second != sid ) {
        ++k;
        _lock_vt.unlock();
        goto try_next;
      }

      for ( vtime::vtime_type::const_iterator i = vt.vt.begin(); i != vt.vt.end(); ++i ) {
        if ( (i->first == k->src()) && (i->first.second != sid) ) {
          if ( (i->second + 1) != tmp[k->src()] ) {
            ++k;
            _lock_vt.unlock();
            goto try_next;
          }
        } else if ( i->second < tmp[i->first] ) {
          ++k;
          _lock_vt.unlock();
          goto try_next;
        }
      }

      // misc::use_syslog<LOG_INFO,LOG_USER>() << "ove.pop_back" << ':' << __FILE__ << ':' << __LINE__ << ':' << sid << ':' << k->value().ev.code() << endl;

      ++vt[k->src()];
      _lock_vt.unlock();

      ev = k->value().ev;
      ev.src( k->src() );
      ev.dest( k->dest() );

      ove.erase( k++ );
      
      if ( ev.code() != VS_UPDATE_VIEW && ev.code() != VS_LOCK_VIEW ) {
        this->vs_pub_rec( ev );
      }
      // basic_vs::sync_call( ev );
      this->Dispatch( ev );

      delayed_process = true;
      break;

      try_next:
      ;
    }
  } while ( delayed_process );
}

void basic_vs::add_lock_safety()
{
  // belay: avoid infinite lock
  Event_base<CronEntry> cr( EV_EDS_CRON_ADD );
  const stem::EventVoid cr_ev( VS_LOCK_SAFETY );
  cr_ev.dest( make_pair( stem::EventHandler::domain(), sid ) );
  cr_ev.src( make_pair( stem::EventHandler::domain(), sid ) );
  cr_ev.pack( cr.value().ev );

  cr.dest( make_pair( stem::EventHandler::domain(), _cron->self_id() ) );
  cr.value().start = get_system_time() + vs_pub_lock_timeout();
  cr.value().n = 1;
  cr.value().period = 0;
  
  cr.src( make_pair( stem::EventHandler::domain(), sid ) );
  Forward( cr );
}

void basic_vs::vs_lock_safety( const stem::EventVoid& ev )
{
  cerr << HERE << endl;
  if ( ev.src().second != sid ) {
    return;
  }
  cerr << HERE << endl;
  // misc::use_syslog<LOG_INFO,LOG_USER>() << "vs_lock_safety1:" << sid << endl;

  check_remotes();

  if ( lock_addr.second != sid ) {
    cerr << HERE << endl;
#if 0 // review
    if ( !is_avail(lock_addr) ) {
      lock_addr = stem::badaddr;
      vs_lock_safety_sequental_attempts = 0;
      RemoveState( VS_ST_LOCKED );
      lock_rsp.clear();
      process_delayed();
    } else {
      add_lock_safety();
    }
#else // review
    add_lock_safety();
#endif // review
    return;
  }

  // misc::use_syslog<LOG_INFO,LOG_USER>() << "vs_lock_safety2:" << sid << endl;
  
  cerr << HERE << endl;
  check_lock_rsp();

  if ( ++vs_lock_safety_sequental_attempts < max_vs_lock_safety_sequental_attempts ) {
    cerr << HERE << endl;
    add_lock_safety();
  } else {
    cerr << HERE << ' ' << lock_rsp.size() << ' ' << fq.size() << hex << fq.front().code() << dec << endl;
    lock_addr = stem::extbadaddr;
    vs_lock_safety_sequental_attempts = 0;
    RemoveState( VS_ST_LOCKED );
    lock_rsp.clear();
    process_delayed();
    misc::use_syslog<LOG_INFO,LOG_USER>() << HERE << ":unexpected: " << sid << " too long wait in locked state" << endl;
  }
}


bool basic_vs::check_remotes()
{
#if 0 // this function should be removed
  for ( access_container_type::iterator i = remotes_.begin(); i != remotes_.end(); ) {
    if ( !(*i)->is_open() || (*i)->bad() ) {
      delete *i;
      remotes_.erase( i++ );
    } else {
      ++i;
    }
  }

  lock_guard<recursive_mutex> lk( _lock_vt );

  bool drop = false;

  for ( vtime::vtime_type::iterator i = vt.vt.begin(); i != vt.vt.end(); ) {
#if 0 // review
    if ( !is_avail( i->first ) ) {
      points.erase( i->first );
      vt.vt.erase( i++ );
      drop = true;
    } else {
      ++i;
    }
#else // review
    ++i;
#endif // review
  }

  return !drop;
#else // 0
  return true;
#endif // 0
}

void basic_vs::access_points_refresh()
{
#if 0 // review
  this->pub_access_point(); // user-defined

  pair<vs_points::points_type::const_iterator,vs_points::points_type::const_iterator> range = points.equal_range( sid );

  stem::Event_base<janus::detail::access_points> my( VS_ACCESS_POINT_PRI );

  for ( ; range.first != range.second; ++range.first ) {
    my.value().points.insert( *range.first );
  }

  send_to_vsg( my );
#endif // review
}

void basic_vs::pub_access_point()
{
  // fill own access point
}

void basic_vs::access_points_refresh_pri( const stem::Event_base<janus::detail::access_points>& ev )
{
#if 0 // review
  // points.clear(); ?

  {
    lock_guard<recursive_mutex> lk( _lock_vt );

    vtime::vtime_type::const_iterator i = vt.vt.find( ev.src() );

    if ( i == vt.vt.end() ) {
      return;
    }
  }

  this->pub_access_point(); // user-defined

  pair<vs_points::points_type::const_iterator,vs_points::points_type::const_iterator> range = points.equal_range( sid );

  stem::Event_base<janus::detail::access_points> my( VS_ACCESS_POINT_SEC );

  for ( ; range.first != range.second; ++range.first ) {
    my.value().points.insert( *range.first );
  }

  send_to_vsg( my );

  range = ev.value().points.equal_range( ev.src() );

  for ( ; range.first != range.second; ++range.first ) {
    points.insert( *range.first );
  }
#endif // review
}

void basic_vs::access_points_refresh_sec( const stem::Event_base<janus::detail::access_points>& ev )
{
  if ( ev.src().second == sid ) {
    return;
  }

  {
    lock_guard<recursive_mutex> lk( _lock_vt );

    vtime::vtime_type::const_iterator i = vt.vt.find( ev.src() );

    if ( i == vt.vt.end() ) {
      return;
    }
  }

#if 0 // review
  pair<vs_points::points_type::const_iterator,vs_points::points_type::const_iterator> range = ev.value().points.equal_range( ev.src() );
  for ( ; range.first != range.second; ++range.first ) {
    vs_points::access_t& p = points.insert( make_pair(ev.src(), vs_points::access_t()) )->second;
    p = range.first->second;
  }
#endif // review
}

void basic_vs::vs_access_point( const stem::Event_base< vs_points >& ev )
{
#if 0 // review
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
#endif // review
}

xmt::uuid_type basic_vs::flush_id( const stem::Event& ev )
{
  if ( ev.code() == VS_FLUSH_RQ ) {
    stem::Event_base<xmt::uuid_type> fev;
    fev.unpack( ev );
    return fev.value();
  }

  return xmt::nil_uuid;
}

const stem::state_type basic_vs::VS_ST_LOCKED = 0x10000;

DEFINE_RESPONSE_TABLE( basic_vs )
  EV_Event_base_T_( ST_NULL, VS_EVENT, vs_process, vs_event )

  EV_Event_base_T_( ST_NULL, VS_JOIN_RQ, vs_join_request, vs_join_rq )
  EV_Event_base_T_( VS_ST_LOCKED, VS_JOIN_RQ, vs_join_request_lk, vs_join_rq )

  EV_Event_base_T_( ST_NULL, VS_FLUSH_RQ, vs_flush_request, xmt::uuid_type )
  EV_Event_base_T_( VS_ST_LOCKED, VS_FLUSH_RQ, vs_flush_request_lk, xmt::uuid_type )

  EV_Event_base_T_( VS_ST_LOCKED, VS_JOIN_RS, vs_group_points, vs_points )

  EV_Event_base_T_( ST_NULL, VS_LOCK_VIEW, vs_lock_view, void )
  EV_Event_base_T_( VS_ST_LOCKED, VS_LOCK_VIEW, vs_lock_view_lk, void )

  EV_Event_base_T_( ST_NULL, VS_LOCK_VIEW_ACK, vs_lock_view_ack, void )

  EV_Event_base_T_( VS_ST_LOCKED, VS_UPDATE_VIEW, vs_update_view, vs_event )

  EV_Event_base_T_( VS_ST_LOCKED, VS_LOCK_SAFETY, vs_lock_safety, void )

  EV_Event_base_T_( ST_NULL, VS_ACCESS_POINT_PRI, access_points_refresh_pri, janus::detail::access_points )
  EV_Event_base_T_( ST_NULL, VS_ACCESS_POINT_SEC, access_points_refresh_sec, janus::detail::access_points )

  EV_Event_base_T_( ST_NULL, VS_ACCESS_POINT, vs_access_point, vs_points )
END_RESPONSE_TABLE

} // namespace janus
