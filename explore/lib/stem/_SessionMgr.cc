// -*- C++ -*- Time-stamp: <99/09/03 14:12:53 ptr>

#ident "$SunId$ %Q%"

#ifdef _MSC_VER
#pragma warning( disable : 4804 )
#endif

#ifdef WIN32
#  ifdef _DLL
#    define __EDS_DLL __declspec( dllexport )
#  else
#    define __EDS_DLL
#  endif
#else
#  define __EDS_DLL
#endif

#include <stdlib.h>

#include <SessionMgr.h>
#include <EDSEv.h>

#if defined( WIN32 ) && defined( _MSC_VER )
#  undef __EDS_DLL_EXPORT
#  define __EDS_DLL_EXPORT __EDS_DLL
#endif

#ifdef WIN32
#define mrand48 rand // rand is bad generator, but wins has no other...
#endif

namespace EDS {

__EDS_DLL
void SessionRsp::pack( std::ostream& s ) const
{
  __pack( s, key );
  __pack( s, addr );
}

__EDS_DLL
void SessionRsp::net_pack( std::ostream& s ) const
{
  __net_pack( s, key );
  __net_pack( s, addr );
}

__EDS_DLL
void SessionRsp::unpack( std::istream& s )
{
  __unpack( s, key );
  __unpack( s, addr );
}

__EDS_DLL
void SessionRsp::net_unpack( std::istream& s )
{
  __net_unpack( s, key );
  __net_unpack( s, addr );
}

void SessionMgr::establish_session( const Event& ev )
{
  stringstream s( ev.value() );
  string account;
  string passwd;

#ifndef _MSC_VER
  getline( s, account );
  getline( s, passwd );
#else
  char ch;
  while ( s.get( ch ).good() && ch != '\n' ) {
    account += ch;
  }
  while ( s.get( ch ).good() && ch != '\n' ) {
    passwd += ch;
  }
#endif
  // check account and permissions, create session leader (or 0)
  EventHandler *_session_leader = session_leader( account, passwd, ev.src() );
  Event_base<SessionRsp> rs( EV_EDS_RS_SESSION );
  rs.dest( ev.src() );
  if ( _session_leader != 0 ) {
    key_type k = key_generate();  // generate new session id

    __S _s;

    _s.leader = _session_leader;  // store session leader
    _s.timeout = time( 0 ) + 20 * 60;

    Container::iterator i = find_if( _M_c.begin(), _M_c.end(),
                                     compose1( bind2nd( _eq_key, -1 ), _skey ) );
    if ( i == _M_c.end() ) {
      _M_c.push_back( account_type(k,_s) );
    } else {
      (*i).first = k;
      (*i).second = _s;
    }
    rs.value().key = k;
    rs.value().addr = _session_leader->self_id();
    _session_leader->Send( Event_convert<SessionRsp>()(rs) );
  } else {
    rs.value().key = static_cast<key_type>(-1);
    rs.value().addr = static_cast<Event::key_type>(Event::badaddr);
    Send( Event_convert<SessionRsp>()(rs) );    
  }
}

void SessionMgr::restore_session( const Event_base<SessionMgr::key_type>& ev )
{
  key_type k = ev.value();
  Container::iterator i = find_if( _M_c.begin(), _M_c.end(),
                                   compose1( bind2nd( _eq_key, k ), _skey ) );

  Event_base<SessionRsp> rs( EV_EDS_RS_SESSION );
  rs.dest( ev.src() );
  if ( i != _M_c.end() ) {
    rs.value().key = k;
    rs.value().addr = _sess(*i).leader->self_id();
    _sess(*i).leader->Send( Event_convert<SessionRsp>()(rs) );    
  } else {
    rs.value().key = static_cast<key_type>(-1);
    rs.value().addr = static_cast<Event::key_type>(Event::badaddr);
    Send( Event_convert<SessionRsp>()(rs) );
  }
}

void SessionMgr::close_session( const Event_base<SessionMgr::key_type>& ev )
{
  Container::iterator i = find_if( _M_c.begin(), _M_c.end(),
                                   compose1( bind2nd( _eq_key, ev.value() ), _skey ) );
  if ( i != _M_c.end() ) {
    destroy_session_leader( _sess(*i).leader );
    (*i).first = static_cast<key_type>(-1);
    (*i).second.leader = 0;
  }
}

SessionMgr::key_type SessionMgr::key_generate()
{
  key_type k;
  do {
    while ( (k = static_cast<key_type>(mrand48())) == -1 );
  } while ( find_if( _M_c.begin(), _M_c.end(),
                     compose1( bind2nd( _eq_key, k ), _skey ) ) != _M_c.end() );
  return k;
}

__EDS_DLL
EventHandler *SessionMgr::session_leader( const string&, const string&,
                                          Event::key_type ) throw ()
{
  return 0;
}

__EDS_DLL
void SessionMgr::destroy_session_leader( EventHandler * ) throw ()
{
}

DEFINE_RESPONSE_TABLE( SessionMgr )
  EV_EDS(ST_NULL,EV_EDS_RQ_SESSION,establish_session)
  EV_Event_base_T_(ST_NULL,EV_EDS_RRQ_SESSION,restore_session,key_type)
  EV_Event_base_T_(ST_NULL,EV_EDS_CL_SESSION,close_session,key_type)
END_RESPONSE_TABLE

} // namespace EDS
