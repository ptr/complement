// -*- C++ -*- Time-stamp: <09/09/09 13:36:28 ptr>

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

void VSsync_rq::pack( std::ostream& s ) const
{
  __pack( s, grp );
  __pack( s, mess );
}

void VSsync_rq::unpack( std::istream& s )
{
  __unpack( s, grp );
  __unpack( s, mess );
}

void VSsync::pack( std::ostream& s ) const
{
  gvt.pack( s );
  VSsync_rq::pack( s );
}

void VSsync::unpack( std::istream& s )
{
  gvt.unpack( s );
  VSsync_rq::unpack( s );
}

void VSmess::pack( std::ostream& s ) const
{
  __pack( s, code );
  __pack( s, src );
  VSsync::pack( s );
}

void VSmess::unpack( std::istream& s )
{
  __unpack( s, code );
  __unpack( s, src );
  VSsync::unpack( s );
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

namespace detail {

bool vtime_obj_rec::deliver( const VSmess& m )
{
  if ( order_correct( m ) ) {
    lvt[m.src] += m.gvt;
    lvt[m.src][m.grp][m.src] = vt.gvt[m.grp][m.src] + 1;
    vt.gvt[m.grp].sup( lvt[m.src][m.grp] );
    return true;
  }

  return false;
}

bool vtime_obj_rec::deliver_delayed( const VSmess& m )
{
  if ( order_correct_delayed( m ) ) {
    lvt[m.src] += m.gvt;
    lvt[m.src][m.grp][m.src] = vt.gvt[m.grp][m.src] + 1;
    vt.gvt[m.grp].sup( lvt[m.src][m.grp] );
    return true;
  }

  return false;
}

bool vtime_obj_rec::order_correct( const VSmess& m )
{
  if ( groups.find( m.grp ) == groups.end() ) {
    throw domain_error( "virtual synchrony object not member of group" );
  }

  gvtime gvt( m.gvt );

  if ( (vt.gvt[m.grp][m.src] + 1) != gvt[m.grp][m.src] ) {
    if ( (vt.gvt[m.grp][m.src] + 1) > gvt[m.grp][m.src] ) {
      throw out_of_range( "duplicate or wrong virtual synchrony message" );
    }
    return false;
  }

  vtime xvt = lvt[m.src][m.grp] + gvt[m.grp];
  xvt[m.src] = 0; // force exclude message originator, it checked above

  if ( !(xvt <= vt[m.grp]) ) {
    return false;
  }

  // check side casuality (via groups other then message's group)
  for ( groups_container_type::const_iterator l = groups.begin(); l != groups.end(); ++l ) {
    if ( (*l != m.grp) && !((lvt[m.src][*l] + gvt[*l]) <= vt[*l]) ) {
      return false;
    }
  }

  return true;
}

ostream& vtime_obj_rec::trace_deliver( const VSmess& m, ostream& o )
{
  if ( groups.find( m.grp ) == groups.end() ) {
    return o << "virtual synchrony object not member of group";
  }

  gvtime gvt( m.gvt );

  if ( (vt.gvt[m.grp][m.src] + 1) != gvt[m.grp][m.src] ) {
    if ( (vt.gvt[m.grp][m.src] + 1) > gvt[m.grp][m.src] ) {
      return o << "duplicate or wrong VT message, " << vt.gvt << " vs " << gvt;
    }
    return o << "counter violation, " << vt.gvt << " vs " << gvt;
  }

  vtime xvt = lvt[m.src][m.grp] + gvt[m.grp];
  xvt[m.src] = 0; // force exclude message originator, it checked above

  if ( !(xvt <= vt[m.grp]) ) {
    return o << "casuality violation, " << xvt << " vs " << vt[m.grp];
  }

  // check side casuality (via groups other then message's group)
  for ( groups_container_type::const_iterator l = groups.begin(); l != groups.end(); ++l ) {
    if ( (*l != m.grp) && !((lvt[m.src][*l] + gvt[*l]) <= vt[*l]) ) {
      return o << "side casuality violation, " << (lvt[m.src][*l] + gvt[*l]) << " vs " << vt[*l];
    }
  }

  return o << "should be delivered";
}

bool vtime_obj_rec::order_correct_delayed( const VSmess& m )
{
  gvtime gvt( m.gvt );

  if ( (vt.gvt[m.grp][m.src] + 1) != gvt[m.grp][m.src] ) {
    return false;
  }

  vtime xvt = lvt[m.src][m.grp] + gvt[m.grp];
  xvt[m.src] = 0; // force exclude message originator, it checked above

  if ( !(xvt <= vt[m.grp]) ) {
    return false;
  }

  // check side casuality (via groups other then message's group)
  for ( groups_container_type::const_iterator l = groups.begin(); l != groups.end(); ++l ) {
    if ( (*l != m.grp) && !((lvt[m.src][*l] + gvt[*l]) <= vt[*l]) ) {
      return false;
    }
  }

  return true;
}

void vtime_obj_rec::delta( gvtime& vtstamp, const janus::addr_type& from, const janus::addr_type& to, gid_type grp )
{
  vtstamp = vt - svt[to]; // delta
  vtstamp[grp][from] = vt.gvt[grp][from]; // my counter, as is, not delta
}

bool vtime_obj_rec::rm_group( gid_type g )
{
  // strike out group g from my groups list
  groups_container_type::iterator i = groups.find( g );
  if ( i == groups.end() ) {
    throw domain_error( "virtual synchrony object not member of group" );
  }
  groups.erase( i );

  // remove my VT component for group g
  gvtime::gvtime_type::iterator j = vt.gvt.find( g );

  if ( j != vt.gvt.end() ) {
    vt.gvt.erase( j );
  }

  // remove sended VT components for group g
  for ( snd_delta_vtime_t::iterator k = svt.begin(); k != svt.end(); ++k ) {
    gvtime::gvtime_type::iterator l = k->second.gvt.find( g );
    if ( l != k->second.gvt.end() ) {
      k->second.gvt.erase( l );
    }
  }

  // remove recieved VT components for group g
  for ( delta_vtime_type::iterator k = lvt.begin(); k != lvt.end(); ++k ) {
    gvtime::gvtime_type::iterator l = k->second.gvt.find( g );
    if ( l != k->second.gvt.end() ) {
      k->second.gvt.erase( l );
    }
  }

  // remove messages for group g that wait in delay pool
  for ( dpool_t::iterator p = dpool.begin(); p != dpool.end(); ) {
    if ( p->second->value().grp == g ) {
      dpool.erase( p++ );
    } else {
      ++p;
    }
  }

  return groups.empty() ? true : false;
}

void vtime_obj_rec::rm_member( const janus::addr_type& addr )
{
  delta_vtime_type::iterator i = lvt.find( addr );

  if ( i != lvt.end() ) {
    lvt.erase( i );
  }

  snd_delta_vtime_t::iterator j = svt.find( addr );

  if ( j != lvt.end() ) {
    svt.erase( j );
  }
}

void vtime_obj_rec::sync( gid_type g, const janus::addr_type& addr, const gvtime::gvtime_type& gvt )
{
  lvt[addr] = gvt;
  gvtime::gvtime_type::const_iterator i = gvt.find( g );
  if ( i != gvt.end() ) {
    vt.gvt[g].sup( i->second.vt );
    // vtime_type::const_iterator j = i->second.vt.find( oid );
    // if ( j != i->second.vt.end() ) {
    //   vt.gvt[g][oid] = j->second;
    //   cerr << "**** " << gvt << endl;
    // }
  }
}

} // namespace detail


char *Init_buf[128];
Janus *VTHandler::_vtdsp = 0;
static int *_rcount = 0;

void VTHandler::Init::__at_fork_prepare()
{
}

void VTHandler::Init::__at_fork_child()
{
  if ( *_rcount != 0 ) {
    // delete VTHandler::_vtdsp->_hostmgr;
    // VTHandler::_vtdsp->_hostmgr = new VSHostMgr( "vshostmgr" );
  }
}

void VTHandler::Init::__at_fork_parent()
{
}

void VTHandler::Init::_guard( int direction )
{
  static std::tr2::recursive_mutex _init_lock;

  std::tr2::lock_guard<std::tr2::recursive_mutex> lk(_init_lock);
  static int _count = 0;

  if ( direction ) {
    if ( _count++ == 0 ) {
#ifdef _PTHREADS
      _rcount = &_count;
      pthread_atfork( __at_fork_prepare, __at_fork_parent, __at_fork_child );
#endif
      // VTHandler::_vtdsp = new Janus( xmt::uid(), "janus" );
      // VTHandler::_vtdsp->_hostmgr = new VSHostMgr( "vshostmgr" );
    }
  } else {
    --_count;
    if ( _count == 1 ) { // 0+1 due to _hostmgr
      // delete VTHandler::_vtdsp;
      // VTHandler::_vtdsp = 0;
    }
  }
}

VTHandler::Init::Init()
{ _guard( 1 ); }

VTHandler::Init::~Init()
{ _guard( 0 ); }

void VTHandler::JaSend( const stem::Event& ev )
{
  ev.src( self_id() );
  // _vtdsp->JaSend( ev, ev.dest() ); // throw domain_error, if not group member
}

VTHandler::VTHandler() :
   EventHandler()
{
  new( Init_buf ) Init();
}

VTHandler::VTHandler( const char *info ) :
   EventHandler( info )
{
  new( Init_buf ) Init();
}

VTHandler::VTHandler( stem::addr_type id, const char *info ) :
   EventHandler( id, info )
{
  new( Init_buf ) Init();
}

VTHandler::~VTHandler()
{
  // Unsubscribe();

  ((Init *)Init_buf)->~Init();
}

void VTHandler::Unsubscribe()
{
  // _vtdsp->Unsubscribe( oid_type( self_id() ) );
}

void VTHandler::JoinGroup( gid_type grp )
{
  // _vtdsp->Subscribe( self_id(), oid_type( self_id() ), grp );
}

void VTHandler::LeaveGroup( gid_type grp )
{
  // _vtdsp->Unsubscribe( oid_type( self_id() ), grp );
}

void VTHandler::VSNewMember( const stem::Event_base<VSsync_rq>& ev )
{
  stem::Event_base<VSsync> out_ev( VS_SYNC_TIME );
  out_ev.dest( ev.src() );
  out_ev.value().grp = ev.value().grp;
  get_gvtime( ev.value().grp, out_ev.value().gvt.gvt );
#ifdef __FIT_VS_TRACE
#if 0
  try {
    std::tr2::lock_guard<std::tr2::mutex> lk(_vtdsp->_lock_tr);
    if ( _vtdsp->_trs != 0 && _vtdsp->_trs->good() && (_vtdsp->_trflags & Janus::tracegroup) ) {
      *_vtdsp->_trs << " -> VS_SYNC_TIME G" << ev.value().grp << " "
                    << hex << showbase
                    << self_id() << " -> " << out_ev.dest() << dec << endl;
    }
  }
  catch ( ... ) {
  }
#endif
#endif // __FIT_VS_TRACE
  Send( out_ev );
}

void VTHandler::VSNewMember_data( const stem::Event_base<VSsync_rq>& ev, const string& data )
{
  stem::Event_base<VSsync> out_ev( VS_SYNC_TIME );
  out_ev.dest( ev.src() );
  out_ev.value().grp = ev.value().grp;
  get_gvtime( ev.value().grp, out_ev.value().gvt.gvt );
  out_ev.value().mess = data;
#ifdef __FIT_VS_TRACE
#if 0
  try {
    std::tr2::lock_guard<std::tr2::mutex> lk(_vtdsp->_lock_tr);
    if ( _vtdsp->_trs != 0 && _vtdsp->_trs->good() && (_vtdsp->_trflags & Janus::tracegroup) ) {
      *_vtdsp->_trs << " -> VS_SYNC_TIME (data) G" << ev.value().grp << " "
                    << hex << showbase
                    << self_id() << " -> " << out_ev.dest() << dec << endl;
    }
  }
  catch ( ... ) {
  }
#endif
#endif // __FIT_VS_TRACE
  Send( out_ev );
}

void VTHandler::get_gvtime( gid_type g, gvtime::gvtime_type& gvt )
{
  // _vtdsp->get_gvtime( g, self_id(), gvt );
}

void VTHandler::VSOutMember( const stem::Event_base<VSsync_rq>& ev )
{
#ifdef __FIT_VS_TRACE
#if 0
  try {
    std::tr2::lock_guard<std::tr2::mutex> lk(_vtdsp->_lock_tr);
    if ( _vtdsp->_trs != 0 && _vtdsp->_trs->good() && (_vtdsp->_trflags & Janus::tracegroup) ) {
      *_vtdsp->_trs << "<-  VS_OUT_MEMBER G" << ev.value().grp
                    << hex << showbase
                    << ev.src() << " -> " << ev.dest() << dec << endl;
    }
  }
  catch ( ... ) {
  }
#endif
#endif // __FIT_VS_TRACE
}

void VTHandler::VSsync_time( const stem::Event_base<VSsync>& ev )
{
  try {
    // sync data from ev.value().mess
#ifdef __FIT_VS_TRACE
#if 0
    try {
      std::tr2::lock_guard<std::tr2::mutex> lk(_vtdsp->_lock_tr);
      if ( _vtdsp->_trs != 0 && _vtdsp->_trs->good() && (_vtdsp->_trflags & Janus::tracegroup) ) {
        *_vtdsp->_trs << "<-  VS_SYNC_TIME G" << ev.value().grp << " "
                      << hex << showbase
                      << ev.src() << " -> " << ev.dest() << dec << endl;
      }
    }
    catch ( ... ) {
    }
#endif
#endif // __FIT_VS_TRACE
    // _vtdsp->set_gvtime( ev.value().grp, self_id(), ev.value().gvt.gvt );
  }
  catch ( const domain_error&  ) {
  }
}

void VTHandler::VSMergeRemoteGroup( const stem::Event_base<VSsync_rq>& e )
{
  stem::Event_base<VSsync> out_ev( VS_SYNC_GROUP_TIME );
  out_ev.dest( e.src() );
  out_ev.value().grp = e.value().grp;
  get_gvtime( e.value().grp, out_ev.value().gvt.gvt );
#ifdef __FIT_VS_TRACE
#if 0
  try {
    std::tr2::lock_guard<std::tr2::mutex> lk(_vtdsp->_lock_tr);
    if ( _vtdsp->_trs != 0 && _vtdsp->_trs->good() && (_vtdsp->_trflags & Janus::tracegroup) ) {
      *_vtdsp->_trs << " -> VS_SYNC_GROUP_TIME G" << e.value().grp << " "
                    << hex << showbase
                    << self_id() << " -> " << out_ev.dest() << dec << endl;
    }
  }
  catch ( ... ) {
  }
#endif
#endif // __FIT_VS_TRACE
  Send( out_ev );
}

void VTHandler::VSMergeRemoteGroup_data( const stem::Event_base<VSsync_rq>& e, const string& data )
{
  stem::Event_base<VSsync> out_ev( VS_SYNC_GROUP_TIME );
  out_ev.dest( e.src() );
  out_ev.value().grp = e.value().grp;
  get_gvtime( e.value().grp, out_ev.value().gvt.gvt );
  out_ev.value().mess = data;
#ifdef __FIT_VS_TRACE
#if 0
  try {
    std::tr2::lock_guard<std::tr2::mutex> lk(_vtdsp->_lock_tr);
    if ( _vtdsp->_trs != 0 && _vtdsp->_trs->good() && (_vtdsp->_trflags & Janus::tracegroup) ) {
      *_vtdsp->_trs << " -> VS_SYNC_GROUP_TIME (data) G" << e.value().grp << " "
                    << hex << showbase
                    << self_id() << " -> " << out_ev.dest() << dec << endl;
    }
  }
  catch ( ... ) {
  }
#endif
#endif // __FIT_VS_TRACE
  Send( out_ev );
}


DEFINE_RESPONSE_TABLE( VTHandler )
  EV_Event_base_T_( ST_NULL, VS_NEW_MEMBER, VSNewMember, VSsync_rq )
  EV_Event_base_T_( ST_NULL, VS_OUT_MEMBER, VSOutMember, VSsync_rq )
  EV_Event_base_T_( ST_NULL, VS_SYNC_TIME, VSsync_time, VSsync )
  EV_Event_base_T_( ST_NULL, VS_MERGE_GROUP, VSMergeRemoteGroup, VSsync_rq )
END_RESPONSE_TABLE

} // namespace vt

namespace std {

ostream& operator <<( ostream& o, const janus::vtime::vtime_type::value_type& v )
{
  return o << "(" << v.first << "," << v.second << ")";
}

ostream& operator <<( ostream& o, const janus::vtime::vtime_type& v )
{
  for ( janus::vtime::vtime_type::const_iterator i = v.begin(); i != v.end(); ++i ) {
    if ( i != v.begin() ) {
      o << ", ";
    }
    o << *i;
  }
  return o;
}

ostream& operator <<( ostream& o, const janus::vtime& v )
{ return o << v.vt; }

ostream& operator <<( ostream& o, const janus::gvtime::gvtime_type::value_type& v )
{
  o << v.first << ": " << v.second.vt;
}

ostream& operator <<( ostream& o, const janus::gvtime::gvtime_type& v )
{
  o << "{\n";
  for ( janus::gvtime::gvtime_type::const_iterator i = v.begin(); i != v.end(); ++i ) {
    o << "\t" << *i << "\n";
  }
  return o << "}\n";
}

ostream& operator <<( ostream& o, const janus::gvtime& v )
{
  return o << v.gvt;
}

ostream& operator <<( ostream& o, const janus::VSsync& m )
{
  // ios_base::fmtflags f = o.flags( ios_base::hex );
  o << "G" << m.grp << " " << m.gvt;
  
  return o;
}

ostream& operator <<( ostream& o, const janus::VSmess& m )
{
  ios_base::fmtflags f = o.flags( ios_base::hex | ios_base::showbase );
  o << "C" << m.code << dec << " " << m.src << " " << static_cast<const janus::VSsync&>(m);
  o.flags( f );
  return o;
}

} // namespace janus

