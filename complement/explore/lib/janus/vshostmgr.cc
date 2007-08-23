// -*- C++ -*- Time-stamp: <07/08/23 11:24:18 ptr>

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
    VTHandler()
{
  vshost.insert( gaddr_type( stem::janus_addr ) );
  JoinGroup( vshosts_group );
}

VSHostMgr::VSHostMgr( stem::addr_type id, const char *info ) :
    VTHandler( id, info )
{
  vshost.insert( gaddr_type( stem::janus_addr ) );
  JoinGroup( vshosts_group );
}

VSHostMgr::VSHostMgr( const char *info ) :
    VTHandler( info )
{
  // vtdispatcher()->settrf( janus::Janus::tracenet | janus::Janus::tracedispatch | janus::Janus::tracefault | janus::Janus::tracedelayed | janus::Janus::tracegroup );

  vshost.insert( gaddr_type( stem::janus_addr ) );
  JoinGroup( vshosts_group );
}

VSHostMgr::~VSHostMgr()
{
  while ( !_clients.empty() ) {
    _clients.front()->close();
    _clients.front()->join();
    delete _clients.front();
    _clients.pop_front();
  }
  while ( !_servers.empty() ) {
    _servers.front()->close();
    _servers.front()->wait();
    delete _servers.front();
    _servers.pop_front();
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
    vshost.insert( ga );
  }

  VTHandler::VSsync_time(ev);
}

void VSHostMgr::connect( const char *host, int port )
{
  _clients.push_back( new NetTransportMgr() );

  addr_type zero = _clients.back()->open( host, port );

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
    Send( ev );
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
}

void VSHostMgr::serve( int port )
{
  _servers.push_back( new sockmgr_stream_MP<NetTransport>( port ) );
#ifdef __FIT_VS_TRACE
  try {
    scoped_lock lk(vtdispatcher()->_lock_tr);
    if ( vtdispatcher()->_trs != 0 && vtdispatcher()->_trs->good() && (vtdispatcher()->_trflags & Janus::tracenet) ) {
      *vtdispatcher()->_trs << "serve " << port
                            << (_servers.back()->good() ? " ok" : " fail" )
                            << endl;
    }
  }
  catch ( ... ) {
  }
#endif // __FIT_VS_TRACE
}

void VSHostMgr::Subscribe( stem::addr_type addr, oid_type oid, group_type grp )
{
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
        Forward( ev );
#ifdef __FIT_VS_TRACE
        try {
          scoped_lock lk(vtdispatcher()->_lock_tr);
          if ( vtdispatcher()->_trs != 0 && vtdispatcher()->_trs->good() && (vtdispatcher()->_trflags & Janus::tracenet) ) {
            *vtdispatcher()->_trs << " -> VS_NEW_REMOTE_MEMBER G" << grp << " "
                                  << hex << showbase
                                  << ev.src() << " -> " << ev.dest() << dec << endl;
      }
    }
    catch ( ... ) {
    }
#endif // __FIT_VS_TRACE
      }
    }
  }
}

// DEFINE_RESPONSE_TABLE( VSHostMgr )
//   EV_Event_base_T_( ST_NULL, VS_NEW_REMOTE_MEMBER, VSNewRemoteMemberDirect, VSsync_rq )
//   EV_Event_base_T_( ST_NULL, VS_NEW_MEMBER_RV, VSNewRemoteMemberRevert, VSsync_rq )
// END_RESPONSE_TABLE


} // namespace janus
