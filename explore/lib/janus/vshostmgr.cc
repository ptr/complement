// -*- C++ -*- Time-stamp: <07/08/26 12:56:20 ptr>

#include <janus/vshostmgr.h>

#include <iostream>
#include <janus/vtime.h>
#include <janus/janus.h>

#include <stem/EvManager.h>
#include <stem/NetTransport.h>
#include <stem/Event.h>
#include <stem/EvPack.h>
#include <sockios/sockmgr.h>

#include <mt/xmt.h>

#include <list>
#include <sstream>

namespace janus {

using namespace std;
using namespace stem;
using namespace xmt;
using namespace janus;

VSHostMgr::VSHostMgr() :
    VTHandler(),
    finalizer( "vshostmgr aux" )
{
  vshost.insert( gaddr_type( stem::janus_addr ) );
  JoinGroup( vshosts_group );
}

VSHostMgr::VSHostMgr( stem::addr_type id, const char *info ) :
    VTHandler( id, info ),
    finalizer( "vshostmgr aux" )
{
  vshost.insert( gaddr_type( stem::janus_addr ) );
  JoinGroup( vshosts_group );
}

VSHostMgr::VSHostMgr( const char *info ) :
    VTHandler( info ),
    finalizer( "vshostmgr aux" )
{
  // vtdispatcher()->settrf( janus::Janus::tracenet | janus::Janus::tracedispatch | janus::Janus::tracefault | janus::Janus::tracedelayed | janus::Janus::tracegroup );

  vshost.insert( gaddr_type( stem::janus_addr ) );
  JoinGroup( vshosts_group );

  bool is_connected = false;

  xmt::scoped_lock lk( _wknh_lock );
  for ( vs_wellknown_hosts_container_t::const_iterator i = _wknhosts.begin(); i != _wknhosts.end(); ++i ) {
    string s( *i );
    string::size_type p = s.find( ':' );
    if ( p != string::npos ) {
      s.replace( p, 1, 1, ' ' );
      stringstream ss( s );
      int port = 0;
      ss >> s >> port;
      if ( !ss.fail() ) {
        if ( connect( s.c_str(), port ) != 0 ) {
          continue;
        }
        is_connected = true;
        break;
      }
    }
  }
  if ( !is_connected && _srvport != 0 ) {
    serve( _srvport );
  }
}

VSHostMgr::~VSHostMgr()
{
  if ( !_clients.empty() || !_servers.empty() ) {
    VTHandler::Unsubscribe(); // before channels closed!

    stem::EventVoid ev( VS_HOST_MGR_FINAL );

    ev.dest( finalizer.self_id() );
    Send( ev );

    finalizer.wait();

    // give the chance to deliver VS_OUT_MEMBER message to remotes...
    // Do you know better solution, because this one is ugly?
    for ( int i = 0; i < 5; ++i ) {
      xmt::Thread::yield();
      xmt::delay( xmt::timespec( 0, 100000 ) );
    }

    // shutdown clients...
    while ( !_clients.empty() ) {
      _clients.front()->close();
      _clients.front()->join();
      delete _clients.front();
      _clients.pop_front();
    }
    // ... and servers
    while ( !_servers.empty() ) {
      _servers.front()->close();
      _servers.front()->wait();
      delete _servers.front();
      _servers.pop_front();
    }
  }
}

void VSHostMgr::VSNewMember( const stem::Event_base<VSsync_rq>& ev )
{
  // pack vshost, 
  stringstream s;

  gaddr_type ga = manager()->reflect( ev.src() );
  if ( ga != gaddr_type() ) {
    ga.addr = stem::janus_addr;
  }

#ifdef __FIT_VS_TRACE
  try {
    scoped_lock lk(vtdispatcher()->_lock_tr);
    if ( vtdispatcher()->_trs != 0 && vtdispatcher()->_trs->good() && (vtdispatcher()->_trflags & Janus::tracenet) ) {
      *vtdispatcher()->_trs << "VSHostMgr sync (add) " << ga << endl;
    }
  }
  catch ( ... ) {
  }
#endif // __FIT_VS_TRACE
  vshost.insert( ga ); // address of remote Janus

  stem::__pack_base::__net_pack( s, static_cast<uint32_t>(vshost.size()) );
  for ( vshost_container_t::const_iterator i = vshost.begin(); i != vshost.end(); ++i ) {
    i->net_pack( s );
  }
  VTHandler::VSNewMember_data( ev, s.str() );
  // VTHandler::VSNewMember( ev );
}

void VSHostMgr::VSOutMember( const stem::Event_base<VSsync_rq>& ev )
{
  // remove host from vshost
  gaddr_type ga = manager()->reflect( ev.src() );
  if ( ga != gaddr_type() ) {
    ga.addr = stem::janus_addr;
  }

  vshost_container_t::iterator i = vshost.find( ga );
  if ( i != vshost.end() ) {
    vshost.erase( i );
  }
#ifdef __FIT_VS_TRACE
  VTHandler::VSOutMember( ev );
#endif
}

void VSHostMgr::VSsync_time( const stem::Event_base<VSsync>& ev )
{
  // extract ev.value().mess, sync with vshost
  stringstream s( ev.value().mess );

  uint32_t sz;
  gaddr_type ga;

  stem::__pack_base::__net_unpack( s, sz );
  while ( sz-- > 0 ) {
    ga.net_unpack( s );
    // vshost.push_back( ga );
#ifdef __FIT_VS_TRACE
    try {
      scoped_lock lk(vtdispatcher()->_lock_tr);
      if ( vtdispatcher()->_trs != 0 && vtdispatcher()->_trs->good() && (vtdispatcher()->_trflags & Janus::tracenet) ) {
        *vtdispatcher()->_trs << "VSHostMgr sync " << ga << endl;
      }
    }
    catch ( ... ) {
    }
#endif // __FIT_VS_TRACE
    vshost.insert( ga );
  }

  VTHandler::VSsync_time(ev);
}

int VSHostMgr::connect( const char *host, int port )
{
  NetTransportMgr *mgr = new NetTransportMgr();

  addr_type zero = mgr->open( host, port );

  if ( zero == stem::badaddr ) {
    delete mgr;
    return -1;
  }

  _clients.push_back( mgr );

  gaddr_type ga = manager()->reflect( zero );
  if ( ga != gaddr_type() ) {
    ga.addr = stem::janus_addr;
    addr_type a = manager()->reflect( ga );
    if ( a == badaddr ) {
      a = manager()->SubscribeRemote( ga, "janus" );
    }
    stem::Event_base<VSsync_rq> ev( VS_NEW_MEMBER );
    ev.dest( a );
    ev.value().grp = vshosts_group; // special group
#ifdef __FIT_VS_TRACE
    try {
      scoped_lock lk(vtdispatcher()->_lock_tr);
      if ( vtdispatcher()->_trs != 0 && vtdispatcher()->_trs->good() && (vtdispatcher()->_trflags & Janus::tracenet) ) {
        *vtdispatcher()->_trs << "connect " << host << ":" << port << "\n"
                              << " -> VS_NEW_MEMBER G" << vshosts_group << " "
                              << hex << showbase
                              << self_id() << " -> " << ev.dest() << dec << endl;
      }
    }
    catch ( ... ) {
    }
#endif // __FIT_VS_TRACE
    Send( ev );
    return 0;
  }
#ifdef __FIT_VS_TRACE
  else {
    try {
      scoped_lock lk(vtdispatcher()->_lock_tr);
      if ( vtdispatcher()->_trs != 0 && vtdispatcher()->_trs->good() && (vtdispatcher()->_trflags & Janus::tracenet) ) {
        *vtdispatcher()->_trs << "connect " << host << ":" << port << " fail" << endl;
      }
    }
    catch ( ... ) {
    }
  }
#endif // __FIT_VS_TRACE

  delete _clients.back();
  _clients.pop_back();
  return -2;
}

int VSHostMgr::serve( int port )
{
  sockmgr_stream_MP<NetTransport> *mgr = new sockmgr_stream_MP<NetTransport>( port );
 
#ifdef __FIT_VS_TRACE
  try {
    scoped_lock lk(vtdispatcher()->_lock_tr);
    if ( vtdispatcher()->_trs != 0 && vtdispatcher()->_trs->good() && (vtdispatcher()->_trflags & Janus::tracenet) ) {
      *vtdispatcher()->_trs << "serve " << port
                            << (mgr->good() ? " ok" : " fail" ) << endl;
    }
  }
  catch ( ... ) {
  }
#endif // __FIT_VS_TRACE
  if ( !mgr->good() ) {
    delete mgr;
    return -2;
  }
  _servers.push_back( mgr );
  return 0;
}

void VSHostMgr::Subscribe( stem::addr_type addr, group_type grp )
{
  if ( vshost.empty() ) {
    return;
  }
  try {
    manager()->transport( addr );
  }
  catch ( const range_error& ) {
    // only for local object
    stem::Event_base<VSsync_rq> ev( VS_NEW_REMOTE_MEMBER );
    ev.src( addr );
    gaddr_type ga = manager()->reflect( stem::janus_addr );

    for ( vshost_container_t::const_iterator i = vshost.begin(); i != vshost.end(); ++i ) {
      if ( ga != *i ) {
        ev.dest( manager()->reflect( *i ) );
        ev.value().grp = grp;
#ifdef __FIT_VS_TRACE
        try {
          scoped_lock lk(vtdispatcher()->_lock_tr);
          if ( vtdispatcher()->_trs != 0 && vtdispatcher()->_trs->good() && (vtdispatcher()->_trflags & Janus::tracegroup) ) {
            *vtdispatcher()->_trs << " -> VS_NEW_REMOTE_MEMBER G" << grp << " "
                                  << hex << showbase
                                  << ev.src() << " -> " << ev.dest() << dec << endl;
          }
        }
        catch ( ... ) {
        }
#endif // __FIT_VS_TRACE
        Forward( ev );
      }
    }
  }
}

xmt::mutex VSHostMgr::_wknh_lock;
VSHostMgr::vs_wellknown_hosts_container_t VSHostMgr::_wknhosts;
int VSHostMgr::_srvport = 0;

void VSHostMgr::add_wellknown( const char *nm )
{
  xmt::scoped_lock lk( _wknh_lock );
  _wknhosts.insert( string(nm) );
}

void VSHostMgr::add_wellknown( const std::string& nm )
{
  xmt::scoped_lock lk( _wknh_lock );
  _wknhosts.insert( nm );
}

void VSHostMgr::rm_wellknown( const char *nm )
{
  xmt::scoped_lock lk( _wknh_lock );
  vs_wellknown_hosts_container_t::iterator i = _wknhosts.find( string(nm) );
  if ( i != _wknhosts.end() ) {
    _wknhosts.erase( i );
  }
}

void VSHostMgr::rm_wellknown( const std::string& nm )
{
  xmt::scoped_lock lk( _wknh_lock );
  vs_wellknown_hosts_container_t::iterator i = _wknhosts.find( nm );
  if ( i != _wknhosts.end() ) {
    _wknhosts.erase( i );
  }
}

void VSHostMgr::add_srvport( int p )
{
  xmt::scoped_lock lk( _wknh_lock );
  _srvport = p;
}

// DEFINE_RESPONSE_TABLE( VSHostMgr )
//   EV_Event_base_T_( ST_NULL, VS_NEW_REMOTE_MEMBER, VSNewRemoteMemberDirect, VSsync_rq )
//   EV_Event_base_T_( ST_NULL, VS_NEW_MEMBER_RV, VSNewRemoteMemberRevert, VSsync_rq )
// END_RESPONSE_TABLE

DEFINE_RESPONSE_TABLE( VSHostMgr::Finalizer )
  EV_VOID( ST_NULL, VS_HOST_MGR_FINAL, final )
END_RESPONSE_TABLE


} // namespace janus
