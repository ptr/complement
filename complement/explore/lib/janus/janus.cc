// -*- C++ -*- Time-stamp: <07/10/04 01:04:14 ptr>

#include <janus/janus.h>
#include <janus/vshostmgr.h>
#include <stem/EvManager.h>

// #include <iostream>

namespace janus {

using namespace std;
using namespace xmt;
using namespace stem;

Janus::~Janus()
{
  delete _hostmgr;
}

void Janus::settrf( unsigned f )
{
  scoped_lock _x1( _lock_tr );
  _trflags |= f;
}

void Janus::unsettrf( unsigned f )
{
  scoped_lock _x1( _lock_tr );
  _trflags &= (0xffffffff & ~f);
}

void Janus::resettrf( unsigned f )
{
  scoped_lock _x1( _lock_tr );
  _trflags = f;
}

void Janus::cleantrf()
{
  scoped_lock _x1( _lock_tr );
  _trflags = 0;
}

unsigned Janus::trflags() const
{
  scoped_lock _x1( _lock_tr );

  return _trflags;
}

void Janus::settrs( std::ostream *s )
{
  scoped_lock _x1( _lock_tr );
  _trs = s;
}

void Janus::JaDispatch( const stem::Event_base<VSmess>& m )
{
  pair<gid_map_type::const_iterator,gid_map_type::const_iterator> range =
    grmap.equal_range( m.value().grp );

  for ( ; range.first != range.second; ++range.first ) {
    vt_map_type::iterator i = vtmap.find( range.first->second );
    if ( i == vtmap.end() || i->second.stem_addr() == m.src() ) { // not for nobody and not for self
      continue;
    }
    try {
      // check local or remote? i->second.addr
      // if remote, forward it to foreign VTDispatcher?
      // looks, like local source shouldn't be here?
      check_and_send( i->second, m );
    }
    catch ( const out_of_range& err ) {
      try {
        scoped_lock lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
          *_trs << err.what() << " "
                << __FILE__ << ":" << __LINE__ << endl;
        }
      }
      catch ( ... ) {
      }
    }
    catch ( const domain_error& err ) {
      try {
        scoped_lock lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
          *_trs << err.what() << " "
                << __FILE__ << ":" << __LINE__ << endl;
        }
      }
      catch ( ... ) {
      }
    }
  }
}

void Janus::check_and_send( detail::vtime_obj_rec& vt, const stem::Event_base<VSmess>& m )
{
  if ( vt.deliver( m.value() ) ) {
    stem::Event ev( m.value().code );
    ev.dest(vt.stem_addr());
    ev.src(m.src());
    ev.value() = m.value().mess;
#ifdef __FIT_VS_TRACE
    try {
      scoped_lock lk(_lock_tr);
      if ( _trs != 0 && _trs->good() && (_trflags & tracedispatch) ) {
        *_trs << "Deliver " << m.value() << endl;
      }
    }
    catch ( ... ) {
    }
#endif // __FIT_VS_TRACE
    Forward( ev );
    check_and_send_delayed( vt );
  } else {
#ifdef __FIT_VS_TRACE
    try {
      scoped_lock lk(_lock_tr);
      if ( _trs != 0 && _trs->good() && (_trflags & tracedelayed) ) {
        *_trs << "Delayed " << m.value() << endl;
      }
    }
    catch ( ... ) {
    }
#endif // __FIT_VS_TRACE
    vt.dpool.push_back( make_pair( xmt::timespec(xmt::timespec::now), new Event_base<VSmess>(m) ) ); // 0 should be timestamp
  }
}

void Janus::check_and_send_delayed( detail::vtime_obj_rec& vt )
{
  typedef detail::vtime_obj_rec::dpool_t dpool_t;
  bool more;
  do {
    more = false;
    for ( dpool_t::iterator j = vt.dpool.begin(); j != vt.dpool.end(); ) {
      if ( vt.deliver_delayed( j->second->value() ) ) {
        stem::Event evd( j->second->value().code );
        evd.dest(vt.stem_addr());
        evd.src(j->second->src());
        evd.value() = j->second->value().mess;
#ifdef __FIT_VS_TRACE
        try {
          scoped_lock lk(_lock_tr);
          if ( _trs != 0 && _trs->good() && (_trflags & tracedispatch) ) {
            *_trs << "Deliver delayed " << j->second->value() << endl;
          }
        }
        catch ( ... ) {
        }
#endif // __FIT_VS_TRACE
        Forward( evd );
        delete j->second;
        vt.dpool.erase( j++ );
        more = true;
      } else {
#ifdef __FIT_VS_TRACE
        try {
          scoped_lock lk(_lock_tr);
          if ( _trs != 0 && _trs->good() && (_trflags & tracedispatch) ) {
            *_trs << "Remain delayed " << j->second->value()
                  << "\nReason: ";
            vt.trace_deliver( j->second->value(), *_trs ) << endl;
          }
        }
        catch ( ... ) {
        }
#endif // __FIT_VS_TRACE
        ++j;
      }
    }
  } while ( more );
}

// Send VS event within group grp:

void Janus::JaSend( const stem::Event& e, group_type grp )
{
  // This method not called from Dispatch, but work on the same level and in the same
  // scope, so this lock (from stem::EventHandler) required here:
  xmt::recursive_scoped_lock lk( this->_theHistory_lock );

  const pair<gid_map_type::const_iterator,gid_map_type::const_iterator> range =
    grmap.equal_range( grp );

  for ( gid_map_type::const_iterator o = range.first; o != range.second; ++o ) {
    vt_map_type::iterator i = vtmap.find( o->second );
    if ( i != vtmap.end() && i->second.stem_addr() == e.src() ) { // for self
      detail::vtime_obj_rec& vt = i->second;
      const oid_type& from = o->second;
      stem::Event_base<VSmess> m( VS_MESS );
      m.value().src = from; // oid
      m.value().code = e.code();
      m.value().mess = e.value();
      m.value().grp = grp;
      // m.dest( ??? ); // local VT dispatcher?
      m.src( e.src() );

      // This is like VTDispatch, but VT stamp in every message different,
      // in accordance with individual knowlage about object's VT.

      vt.next( from, grp ); // my counter

      for ( gid_map_type::const_iterator g = range.first; g != range.second; ++g ) {
        vt_map_type::iterator k = vtmap.find( g->second );
        if ( k == vtmap.end() || k->second.stem_addr() == m.src() ) { // not for nobody and not for self
          continue;
        }
        try {
          vt.delta( m.value().gvt, from, g->second, grp );

          // check local or remote? i->second.addr
          // if remote, forward it to foreign VTDispatcher?
          try {
            /* const transport tr = */ manager()->transport( k->second.stem_addr() );
            // remote delivery
            gaddr_type ga = manager()->reflect( k->second.stem_addr() );
            if ( ga != gaddr_type() ) {
              ga.addr = stem::janus_addr;
              addr_type a = manager()->reflect( ga );
              if ( a == badaddr ) {
                a = manager()->SubscribeRemote( ga, "janus" );
              }
              m.dest( a );
              Forward( m );
              vt.base_advance(g->second); // store last send VT to obj
            }
          }
          catch ( const range_error& ) {
            // local delivery
            check_and_send( k->second, m );
            vt.base_advance(g->second); // store last send VT to obj
          }
        }
        catch ( const out_of_range& err ) {
          try {
            scoped_lock lk(_lock_tr);
            if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
              *_trs << err.what() << " "
                    << __FILE__ << ":" << __LINE__ << endl;
            }
          }
          catch ( ... ) {
          }
        }
        catch ( const domain_error& err ) {
          try {
            scoped_lock lk(_lock_tr);
            if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
              *_trs << err.what() << " "
                    << __FILE__ << ":" << __LINE__ << endl;
            }
          }
          catch ( ... ) {
          }
        }
      }

      return;
    }
  }

  throw domain_error( "VT object not member of group" ); // Error: not group member
}

void Janus::Subscribe( stem::addr_type addr, const oid_type& oid, group_type grp )
{
  // See comment on top of VTSend above
  xmt::recursive_scoped_lock lk( this->_theHistory_lock );

  pair<gid_map_type::const_iterator,gid_map_type::const_iterator> range =
    grmap.equal_range( grp );

#ifdef __FIT_VS_TRACE
  try {
    scoped_lock lk(_lock_tr);
    if ( _trs != 0 && _trs->good() && (_trflags & tracegroup) ) {
      int f = _trs->flags();
      *_trs << " VS subscribe G" << grp << " "
            << hex << showbase
            << addr << " " << oid << dec << ", group size before "
            << distance( range.first, range.second ) << endl;
#ifdef STLPORT
      _trs->flags( f );
#else
      _trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
    }
  }
  catch ( ... ) {
  }
#endif // __FIT_VS_TRACE

  for ( ; range.first != range.second; ++range.first ) {
    vt_map_type::iterator i = vtmap.find( range.first->second );
    if ( i != vtmap.end() ) {
      stem::Event_base<VSsync_rq> ev( VS_NEW_MEMBER );
      ev.dest( i->second.stem_addr() );
      ev.src( addr );
      ev.value().grp = grp;
#ifdef __FIT_VS_TRACE
      try {
        scoped_lock lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracegroup) ) {
          *_trs << " -> VS_NEW_MEMBER G" << grp << " "
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

  vtmap[oid].add( addr, grp );
  grmap.insert( make_pair(grp,oid) );

  // cerr << "**** " << grp << " " << xmt::getpid() << endl;

  if ( /* (grp != vshosts_group) && */ (_hostmgr != 0) ) {
#ifdef __FIT_VS_TRACE
    try {
      scoped_lock lk(_lock_tr);
      if ( _trs != 0 && _trs->good() && (_trflags & tracegroup) ) {
        *_trs << " ????? " << __FILE__ << ":" << __LINE__ << endl;
      }
    }
    catch ( ... ) {
    }
#endif // __FIT_VS_TRACE
    _hostmgr->Subscribe( addr, grp );
  }
}

void Janus::Unsubscribe( const oid_type& oid, group_type grp )
{
  // See comment on top of VTSend above
  xmt::recursive_scoped_lock lk( this->_theHistory_lock );

  pair<gid_map_type::iterator,gid_map_type::iterator> range =
    grmap.equal_range( grp );

  vt_map_type::iterator i = vtmap.find( oid );
  addr_cache_t addr_cache;

  while ( range.first != range.second ) {
    if ( range.first->second == oid ) {
      grmap.erase( range.first++ );
    } else {
      vt_map_type::iterator j = vtmap.find( range.first->second );
      if ( j != vtmap.end() ) {
        stem::Event_base<VSsync_rq> ev( VS_OUT_MEMBER );
        stem::addr_type addr = j->second.stem_addr();

        // if address is foreign, send VS_OUT_MEMBER to foreign janus
        if ( (addr & stem::extbit) != 0 ) {
          gaddr_type ga = manager()->reflect( addr );
          if ( ga != gaddr_type() ) {
            ga.addr = stem::janus_addr;
          }
          addr = manager()->reflect( ga );
          // send only once, foreign janus will resend to other group members
          // in the same process:
          if ( addr_cache.find(addr) != addr_cache.end() ) {
            ++range.first;
            continue;
          } else {
            addr_cache.insert( addr );
          }
        }
        ev.dest( addr );
        ev.src( i != vtmap.end() ? i->second.stem_addr() : self_id() );
        ev.value().grp = grp;
#ifdef __FIT_VS_TRACE
        try {
          scoped_lock lk(_lock_tr);
          if ( _trs != 0 && _trs->good() && (_trflags & tracegroup) ) {
            *_trs << " -> VS_OUT_MEMBER "
                  << hex << showbase
                  << ev.src() << " -> " << ev.dest() << dec << endl;
          }
        }
        catch ( ... ) {
        }
#endif // __FIT_VS_TRACE
        Forward( ev );
      }
      ++range.first;
    }
  }

  if ( i != vtmap.end() ) {
    if ( i->second.rm_group( grp ) ) { // no groups more
      vtmap.erase( i );
    }
  }

  // if ( /* (grp != vshosts_group) && */ (_hostmgr != 0) ) {
  //   _hostmgr->Unsubscribe( oid, grp );
  // }
}

void Janus::Unsubscribe( const oid_type& oid )
{
  // See comment on top of JaSend above
  xmt::recursive_scoped_lock lk( this->_theHistory_lock );

  vt_map_type::iterator i = vtmap.find( oid );
  if ( i != vtmap.end() ) {
    list<group_type> grp_list;
    i->second.groups_list( back_inserter( grp_list ) );
    for ( list<group_type>::const_iterator grp = grp_list.begin(); grp != grp_list.end(); ++grp ) {

      pair<gid_map_type::iterator,gid_map_type::iterator> range =
        grmap.equal_range( *grp );

      addr_cache_t addr_cache;

      while ( range.first != range.second ) {
        if ( range.first->second == oid ) {
          grmap.erase( range.first++ );
        } else {
          vt_map_type::iterator j = vtmap.find( range.first->second );
          if ( j != vtmap.end() ) {
            stem::Event_base<VSsync_rq> ev( VS_OUT_MEMBER );
            stem::addr_type addr = j->second.stem_addr();

            // if address is foreign, send VS_OUT_MEMBER to foreign janus
            if ( (addr & stem::extbit) != 0 ) {
              gaddr_type ga = manager()->reflect( addr );
              if ( ga != gaddr_type() ) {
                ga.addr = stem::janus_addr;
              }
              addr = manager()->reflect( ga );
              // send only once, foreign janus will resend to other group members
              // in the same process:
              if ( addr_cache.find(addr) != addr_cache.end() ) {
                ++range.first;
                continue;
              } else {
                addr_cache.insert( addr );
              }
            }
            ev.dest( addr );
            ev.src( i != vtmap.end() ? i->second.stem_addr() : self_id() );
            ev.value().grp = *grp;
#ifdef __FIT_VS_TRACE
            try {
              scoped_lock lk(_lock_tr);
              if ( _trs != 0 && _trs->good() && (_trflags & tracegroup) ) {
                *_trs << " -> VS_OUT_MEMBER "
                      << hex << showbase
                      << ev.src() << " -> " << ev.dest() << dec << endl;
              }
            }
            catch ( ... ) {
            }
#endif // __FIT_VS_TRACE
            Forward( ev );
          }
          ++range.first;
        }
      }
      i->second.rm_group( *grp );
    }
    vtmap.erase( i );
  }

  // if ( grp != vshosts_group ) {
  // }
}

void Janus::get_gvtime( group_type grp, stem::addr_type addr, gvtime_type& gvt )
{
  // See comment on top of JaSend above
  xmt::recursive_scoped_lock lk( this->_theHistory_lock );

  pair<gid_map_type::iterator,gid_map_type::iterator> range =
    grmap.equal_range( grp );
  for ( ; range.first != range.second; ++range.first ) {
    vt_map_type::iterator i = vtmap.find( range.first->second );
    if ( i != vtmap.end() && i->second.stem_addr() == addr ) {
      i->second.get_gvt( gvt );
      return;
    }
  }

  try {
    scoped_lock lk(_lock_tr);
    if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
      *_trs << "virtual synchrony object not member of group" << " " << __FILE__ << ":" << __LINE__ << endl;
    }
  }
  catch ( ... ) {
  }

  throw domain_error( "virtual synchrony object not member of group" ); // Error: not group member
}

void Janus::set_gvtime( group_type grp, stem::addr_type addr, const gvtime_type& gvt )
{
  // See comment on top of JaSend above
  xmt::recursive_scoped_lock lk( this->_theHistory_lock );

  pair<gid_map_type::iterator,gid_map_type::iterator> range =
    grmap.equal_range( grp );
  for ( ; range.first != range.second; ++range.first ) {
    vt_map_type::iterator i = vtmap.find( range.first->second );
    if ( i != vtmap.end() && i->second.stem_addr() == addr ) {
#ifdef __FIT_VS_TRACE
      try {
        scoped_lock lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracegroup) ) {
          *_trs << "Set gvt G" << grp << " " << i->first
                << " (" << addr << ") " << gvt << endl;
        }
      }
      catch ( ... ) {
      }
#endif // __FIT_VS_TRACE
      i->second.sync( grp, i->first, gvt );
      check_and_send_delayed( i->second );
      return;
    }
  }

  try {
    scoped_lock lk(_lock_tr);
    if ( _trs != 0 && _trs->good() && (_trflags & tracefault) ) {
      *_trs << "virtual synchrony object not member of group" << " " << __FILE__ << ":" << __LINE__ << endl;
    }
  }
  catch ( ... ) {
  }

  throw domain_error( "virtual synchrony object not member of group" ); // Error: not group member
}

void Janus::VSNewMember( const stem::Event_base<VSsync_rq>& ev )
{
  if ( ev.value().grp != vshosts_group ) { // shouldn't happens, only trace
#ifdef __FIT_VS_TRACE
    try {
      scoped_lock lk(_lock_tr);
      if ( _trs != 0 && _trs->good() && (_trflags & tracegroup) && (_trflags & tracefault) ) {
        *_trs << "Unexpected VS_NEW_MEMBER on Janus: G" << ev.value().grp << " "
              << hex << showbase
              << ev.src() << " -> " << ev.dest() << dec << endl;
      }
    }
    catch ( ... ) {
    }
#endif // __FIT_VS_TRACE

    return;
  }
  // special group
  // Forward info about remote VS node to VSHostMgr
  ev.dest( _hostmgr->self_id() );
#ifdef __FIT_VS_TRACE
  try {
    scoped_lock lk(_lock_tr);
    if ( _trs != 0 && _trs->good() && (_trflags & tracegroup) ) {
      *_trs << "<-> VS_NEW_MEMBER G" << vshosts_group << " "
            << hex << showbase
            << ev.src() << " -> " << ev.dest() << dec << endl;
    }
  }
  catch ( ... ) {
  }
#endif // __FIT_VS_TRACE
  Forward( ev );

  // Return info about me (VS node) back to remote VS node
  gaddr_type ga = manager()->reflect( ev.src() );
  addr_type janus_addr = badaddr;
  if ( ga != gaddr_type() ) {
    ga.addr = stem::janus_addr;
    janus_addr = manager()->reflect( ga );
    if ( janus_addr == badaddr ) {
      janus_addr = manager()->SubscribeRemote( ga, "janus" );
    }
  }
  stem::Event_base<VSsync_rq> evr( VS_NEW_MEMBER_RV );
  evr.dest( janus_addr );
  evr.value().grp = vshosts_group;
#ifdef __FIT_VS_TRACE
  try {
    scoped_lock lk(_lock_tr);
    if ( _trs != 0 && _trs->good() && (_trflags & tracegroup) ) {
      *_trs << " -> VS_NEW_MEMBER_RV G" << vshosts_group << " "
            << hex << showbase
            << self_id() << " -> " << evr.dest() << dec << endl;
    }
  }
  catch ( ... ) {
  }
#endif // __FIT_VS_TRACE
  Send( evr );
}

void Janus::VSNewRemoteMemberDirect( const stem::Event_base<VSsync_rq>& ev )
{
  if ( ev.value().grp == vshosts_group ) { // special group, and this shouldn't happens
    return;
  }
  group_type grp = ev.value().grp;
  pair<gid_map_type::const_iterator,gid_map_type::const_iterator> range = grmap.equal_range( grp );
  if ( range.first == range.second ) { // no local (?) member within this group
    return;
  }

  const addr_type addr = ev.src();
  gaddr_type ga = manager()->reflect( addr );
  addr_type janus_addr = badaddr;
  if ( ga != gaddr_type() ) {
    ga.addr = stem::janus_addr;
    janus_addr = manager()->reflect( ga );
    if ( janus_addr == badaddr ) {
      janus_addr = manager()->SubscribeRemote( ga, "janus" );
    }
  }
  for ( ; range.first != range.second; ++range.first ) {
    vt_map_type::iterator i = vtmap.find( range.first->second );
    if ( i != vtmap.end() ) {
      // Inform local (?) object about new remote group member
      stem::Event_base<VSsync_rq> evs( VS_NEW_MEMBER );
      evs.dest( i->second.stem_addr() );
      evs.src( addr );
      evs.value().grp = grp;
#ifdef __FIT_VS_TRACE
      try {
        scoped_lock lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracegroup) ) {
          *_trs << " -> VS_NEW_MEMBER (remote) G" << grp << " "
                << hex << showbase
                << evs.src() << " -> " << evs.dest() << dec << endl;
        }
      }
      catch ( ... ) {
      }
#endif // __FIT_VS_TRACE
      Forward( evs );
      // Inform remote object about local (?) group member
      stem::Event_base<VSsync_rq> evr( VS_NEW_MEMBER_RV );
      evr.dest( janus_addr );
      evr.src( i->second.stem_addr() );
      evr.value().grp = grp;
#ifdef __FIT_VS_TRACE
      try {
        scoped_lock lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracegroup) ) {
          *_trs << " -> VS_NEW_MEMBER_RV G" << grp << " "
                << hex << showbase
                << evr.src() << " -> " << evr.dest() << dec << endl;
        }
      }
      catch ( ... ) {
      }
#endif // __FIT_VS_TRACE
      Forward( evr );
    }
  }

  const gaddr_type oid = manager()->reflect( addr ); // ???? oid == gaddr

  vtmap[oid].add( addr, grp );
  grmap.insert( make_pair(grp,oid) );
}

void Janus::VSNewRemoteMemberRevert( const stem::Event_base<VSsync_rq>& ev )
{
  if ( ev.value().grp != vshosts_group ) {
    group_type grp = ev.value().grp;
    pair<gid_map_type::const_iterator,gid_map_type::const_iterator> range = grmap.equal_range( grp );
    if ( range.first != range.second ) { // we have local? member within this group
      const addr_type addr = ev.src();
      const gaddr_type oid = manager()->reflect( addr ); // ???? oid == gaddr
#ifdef __FIT_VS_TRACE
      try {
        scoped_lock lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracegroup) ) {
          int f = _trs->flags();
          *_trs << " VS add remote object " << hex << showbase << oid
                << dec << " G" << grp << endl;
#ifdef STLPORT
          _trs->flags( f );
#else
          _trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
        }
      }
      catch ( ... ) {
      }
#endif // __FIT_VS_TRACE
      // add remote memer of group grp
      vtmap[oid].add( addr, grp );
      grmap.insert( make_pair(grp,oid) );
    }
#ifdef __FIT_VS_TRACE
      else {
      // this VS node has no group grp, so do nothing
      // except trace info
      try {
        scoped_lock lk(_lock_tr);
        if ( _trs != 0 && _trs->good() && (_trflags & tracegroup) && (_trflags & tracefault) ) {
          int f = _trs->flags();
          *_trs << " VS node don't has such group, ignore G" << grp << endl;
#ifdef STLPORT
          _trs->flags( f );
#else
          _trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
        }
      }
      catch ( ... ) {
      }
    }
#endif // __FIT_VS_TRACE
  } else {
    const addr_type addr = ev.src();
    const gaddr_type oid = manager()->reflect( ev.src() ); // ???? oid == gaddr

#ifdef __FIT_VS_TRACE
    try {
      scoped_lock lk(_lock_tr);
      if ( _trs != 0 && _trs->good() && (_trflags & tracegroup) ) {
        int f = _trs->flags();
        *_trs << " VS see node " << hex << showbase << oid << endl;
#ifdef STLPORT
        _trs->flags( f );
#else
        _trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
      }
    }
    catch ( ... ) {
    }
#endif // __FIT_VS_TRACE

    vtmap[oid].add( addr, vshosts_group );
    grmap.insert( make_pair(static_cast<group_type>(vshosts_group),oid) );

    // send request for sync all groups, that present in this VS node:
    stem::Event_base<VSsync_rq> e( VS_MERGE_GROUP );
    e.dest( ev.src() );
    group_cache_t gcache;
    for ( gid_map_type::const_iterator i = grmap.begin(); i != grmap.end(); ++i ) {
      // for all groups, except vshosts_group,
      // and only once for earch group
      if ( (i->first != vshosts_group) && (gcache.find( i->first ) == gcache.end()) ) {
        e.value().grp = i->first;
        Send( e );
        gcache.insert( i->first );
      }
    }
  }
}

void Janus::VSOutMember( const stem::Event_base<VSsync_rq>& ev )
{
#ifdef __FIT_VS_TRACE
  try {
    scoped_lock lk(_lock_tr);
    if ( _trs != 0 && _trs->good() && (_trflags & tracegroup) ) {
      *_trs << "<-  VS_OUT_MEMBER G" << ev.value().grp
            << hex << showbase
            << ev.src() << " -> " << ev.dest() << dec << endl;
    }
  }
  catch ( ... ) {
  }
#endif // __FIT_VS_TRACE

  const gaddr_type oid = manager()->reflect( ev.src() ); // ???? oid == gaddr
  const group_type grp = ev.value().grp;


  pair<gid_map_type::iterator,gid_map_type::iterator> range =
    grmap.equal_range( grp );

  vt_map_type::iterator i = vtmap.find( oid );

  while ( range.first != range.second ) {
    if ( range.first->second == oid ) {
      grmap.erase( range.first++ );
    } else {
      vt_map_type::iterator j = vtmap.find( range.first->second );
      if ( j != vtmap.end() ) {
        stem::addr_type addr = j->second.stem_addr();

        // send only to local addresses
        if ( (addr & stem::extbit) == 0 ) {
          ev.dest( addr );
#ifdef __FIT_VS_TRACE
          try {
            scoped_lock lk(_lock_tr);
            if ( _trs != 0 && _trs->good() && (_trflags & tracegroup) ) {
              *_trs << " -> VS_OUT_MEMBER "
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
      ++range.first;
    }
  }

  if ( i != vtmap.end() ) {
    if ( i->second.rm_group( grp ) ) { // no groups more
      vtmap.erase( i );
    }
  }
}

int Janus::connect( const char *host, int port )
{
  return _hostmgr->connect( host, port );
}

int Janus::serve( int port )
{
  return _hostmgr->serve( port );
}

size_t Janus::vs_known_processes() const
{
  return _hostmgr->vs_known_processes();
}

Janus::difference_type Janus::group_size( group_type grp ) const
{
  // See comment on top of JaSend above
  xmt::recursive_scoped_lock lk( this->_theHistory_lock );

  pair<gid_map_type::const_iterator,gid_map_type::const_iterator> range = grmap.equal_range( grp );

  return distance( range.first, range.second );
}

void Janus::VSMergeRemoteGroup( const stem::Event_base<VSsync_rq>& e )
{
  const group_type grp = e.value().grp;
#ifdef __FIT_VS_TRACE
  try {
    scoped_lock lk(_lock_tr);
    if ( _trs != 0 && _trs->good() && (_trflags & tracegroup) ) {
      int f = _trs->flags();
      *_trs << " VS merge remote group request G" << grp << endl;
#ifdef STLPORT
      _trs->flags( f );
#else
      _trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
    }
  }
  catch ( ... ) {
  }
#endif // __FIT_VS_TRACE
  pair<gid_map_type::const_iterator,gid_map_type::const_iterator> range = grmap.equal_range( grp );
  if ( range.first != range.second ) { // we have local? member within this group
    const addr_type addr = e.src();  // expected that addr correspond to remote Janus

    // gaddr_type ga = manager()->reflect( addr );
    // addr_type janus_addr = badaddr;
    // if ( ga != gaddr_type() ) {
    //   ga.addr = stem::janus_addr;
    //   janus_addr = manager()->reflect( ga );
    //   if ( janus_addr == badaddr ) {
    //     janus_addr = manager()->SubscribeRemote( ga, "janus" );
    //   }
    // }

    // select VS object with latest time, and forward request to it

    // gvtime_type gvt;
    // gvtime_type gvt_last;
    // stem::addr_type addr = stem::badaddr;
    stem::Event_base<VSsync_rq> e_rv( VS_OLD_MEMBER_RV );
    e_rv.dest( /* janus_addr */ addr );
    e_rv.value().grp = grp;

    bool sync = false;

    for ( ; range.first != range.second; ++range.first ) {
      vt_map_type::iterator i = vtmap.find( range.first->second );
      if ( i != vtmap.end() ) {
        stem::addr_type vs_addr = i->second.stem_addr();
        if ( (vs_addr & stem::extbit) == 0 ) {
          e_rv.src( vs_addr );
          Forward( e_rv ); // inform remote janus about group member
          if ( !sync ) {
            // i->second.get_gvt( gvt );
            // if ( gvt_last < gvt ) {
            //   swap( gvt_last, gvt );
            //   addr = i->second.stem_addr();
            // }
            e.dest( vs_addr );
            Forward( e ); // request local VS Handler about request VS_MERGE_GROUP
            // return;
            sync = true; // only one object informed, no need more
          }
        }
      }
    }
    // e.dest( addr );
    // Forward( e );
  }
}

void Janus::VSsync_group_time( const stem::Event_base<VSsync>& ev )
{
  const group_type grp = ev.value().grp;
  const addr_type addr = ev.src();
#ifdef __FIT_VS_TRACE
  try {
    scoped_lock lk(_lock_tr);
    if ( _trs != 0 && _trs->good() && (_trflags & tracegroup) ) {
      int f = _trs->flags();
      *_trs << " VS sync group time from " << hex << showbase << addr
            << dec << " G" << grp << endl;
#ifdef STLPORT
      _trs->flags( f );
#else
      _trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
    }
  }
  catch ( ... ) {
  }
#endif // __FIT_VS_TRACE

  // const gaddr_type oid = manager()->reflect( addr ); // ???? oid == gaddr

  // add remote memer of group grp
  // vtmap[oid].add( addr, grp );
  // grmap.insert( make_pair(grp,oid) );

  // find all local group members and forward data as VS_SYNC_TIME
  pair<gid_map_type::const_iterator,gid_map_type::const_iterator> range = grmap.equal_range( grp );
  stem::Event_base<VSsync> e( VS_SYNC_TIME );
  e.src( addr );
  e.value().grp = grp;
  e.value().gvt.gvt = ev.value().gvt.gvt;
  e.value().mess = ev.value().mess;

  // (stem) address of remote janus:
  gaddr_type ga = manager()->reflect( addr );
  addr_type janus_addr = badaddr;
  if ( ga != gaddr_type() ) {
    ga.addr = stem::janus_addr;
    janus_addr = manager()->reflect( ga );
    if ( janus_addr == badaddr ) {
      janus_addr = manager()->SubscribeRemote( ga, "janus" );
    }
  }

  stem::Event_base<VSsync_rq> e_rv( VS_OLD_MEMBER_RV );
  e_rv.dest( janus_addr );
  e_rv.value().grp = grp;

  for ( ; range.first != range.second; ++range.first ) {
    vt_map_type::iterator i = vtmap.find( range.first->second );
    if ( i != vtmap.end() ) {
      stem::addr_type vs_addr = i->second.stem_addr();
      if ( (vs_addr & stem::extbit) == 0 ) {
        e.dest( vs_addr );
        Forward( e ); // forward VS time from remote as VS_SYNC_TIME
        e_rv.src( vs_addr );
        Forward( e_rv ); // inform remote janus about local group member
      }
    }
  }
}

void Janus::VSOldRemoteMemberRevert( const stem::Event_base<VSsync_rq>& ev )
{
  const group_type grp = ev.value().grp;
  const addr_type addr = ev.src();
  const gaddr_type oid = manager()->reflect( addr ); // ???? oid == gaddr
#ifdef __FIT_VS_TRACE
  try {
    scoped_lock lk(_lock_tr);
    if ( _trs != 0 && _trs->good() && (_trflags & tracegroup) ) {
      int f = _trs->flags();
      *_trs << " VS remote " << hex << showbase << addr
            << " (" << oid << ") "
            << dec << " G" << grp << endl;
#ifdef STLPORT
      _trs->flags( f );
#else
      _trs->flags( static_cast<std::_Ios_Fmtflags>(f) );
#endif
    }
  }
  catch ( ... ) {
  }
#endif // __FIT_VS_TRACE
  // add remote memer of group grp
  vtmap[oid].add( addr, grp );
  grmap.insert( make_pair(grp,oid) );
}


DEFINE_RESPONSE_TABLE( Janus )
  EV_Event_base_T_( ST_NULL, VS_MESS, JaDispatch, VSmess )
  EV_Event_base_T_( ST_NULL, VS_NEW_MEMBER, VSNewMember, VSsync_rq )
  EV_Event_base_T_( ST_NULL, VS_NEW_REMOTE_MEMBER, VSNewRemoteMemberDirect, VSsync_rq )
  EV_Event_base_T_( ST_NULL, VS_NEW_MEMBER_RV, VSNewRemoteMemberRevert, VSsync_rq )
  EV_Event_base_T_( ST_NULL, VS_OLD_MEMBER_RV, VSOldRemoteMemberRevert, VSsync_rq )
  EV_Event_base_T_( ST_NULL, VS_OUT_MEMBER, VSOutMember, VSsync_rq )
  EV_Event_base_T_( ST_NULL, VS_MERGE_GROUP, VSMergeRemoteGroup, VSsync_rq )
  EV_Event_base_T_( ST_NULL, VS_SYNC_GROUP_TIME, VSsync_group_time, VSsync )
END_RESPONSE_TABLE

} // namespace janus
