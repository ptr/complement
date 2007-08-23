// -*- C++ -*- Time-stamp: <07/08/23 12:36:32 ptr>

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

void Janus::Subscribe( stem::addr_type addr, oid_type oid, group_type grp )
{
  // See comment on top of VTSend above
  xmt::recursive_scoped_lock lk( this->_theHistory_lock );

  pair<gid_map_type::const_iterator,gid_map_type::const_iterator> range =
    grmap.equal_range( grp );

  for ( ; range.first != range.second; ++range.first ) {
    vt_map_type::iterator i = vtmap.find( range.first->second );
    if ( i != vtmap.end() ) {
      stem::Event_base<VSsync_rq> ev( VS_NEW_MEMBER );
      ev.dest( i->second.stem_addr() );
      ev.src( addr );
      ev.value().grp = grp;
      Forward( ev );
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
    }
  }

  vtmap[oid].add( addr, grp );
  grmap.insert( make_pair(grp,oid) );

  // cerr << "**** " << grp << " " << xmt::getpid() << endl;

  if ( /* (grp != vshosts_group) && */ (_hostmgr != 0) ) {
    _hostmgr->Subscribe( addr, oid, grp );
  }
}

void Janus::Unsubscribe( oid_type oid, group_type grp )
{
  // See comment on top of VTSend above
  xmt::recursive_scoped_lock lk( this->_theHistory_lock );

  pair<gid_map_type::iterator,gid_map_type::iterator> range =
    grmap.equal_range( grp );

  vt_map_type::iterator i = vtmap.find( oid );
  while ( range.first != range.second ) {
    if ( range.first->second == oid ) {
      grmap.erase( range.first++ );
    } else {
      vt_map_type::iterator j = vtmap.find( range.first->second );
      if ( j != vtmap.end() ) {
        stem::Event_base<VSsync_rq> ev( VS_OUT_MEMBER );
        ev.dest( j->second.stem_addr() );
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

  if ( grp != vshosts_group ) {
  }
}

void Janus::Unsubscribe( oid_type oid )
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

      while ( range.first != range.second ) {
        if ( range.first->second == oid ) {
          grmap.erase( range.first++ );
        } else {
          vt_map_type::iterator j = vtmap.find( range.first->second );
          if ( j != vtmap.end() ) {
            stem::Event_base<VSsync_rq> ev( VS_OUT_MEMBER );
            ev.dest( j->second.stem_addr() );
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
  if ( ev.value().grp == vshosts_group ) {
    ev.dest( _hostmgr->self_id() );
    Forward( ev );
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
    Send( evr );
#ifdef __FIT_VS_TRACE
    try {
      scoped_lock lk(_lock_tr);
      if ( _trs != 0 && _trs->good() && (_trflags & tracegroup) ) {
        *_trs << " -> VS_NEW_MEMBER_RV G" << vshosts_group << " "
              << hex << showbase
              << evr.src() << " -> " << evr.dest() << dec << endl;
      }
    }
    catch ( ... ) {
    }
#endif // __FIT_VS_TRACE

  }
}

void Janus::VSNewRemoteMemberDirect( const stem::Event_base<VSsync_rq>& ev )
{
  if ( ev.value().grp != vshosts_group ) {
    group_type grp = ev.value().grp;
    pair<gid_map_type::const_iterator,gid_map_type::const_iterator> range = grmap.equal_range( grp );
    if ( range.first != range.second ) { // we have local? member within this group
      gaddr_type oid = manager()->reflect( ev.src() ); // ???? oid == gaddr
      addr_type addr = ev.src();
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
          stem::Event_base<VSsync_rq> evs( VS_NEW_MEMBER );
          evs.dest( i->second.stem_addr() );
          evs.src( addr );
          evs.value().grp = grp;
          Forward( evs );
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
          stem::Event_base<VSsync_rq> evr( VS_NEW_MEMBER_RV );
          evr.dest( janus_addr );
          evr.src( i->second.stem_addr() );
          evr.value().grp = grp;
          Forward( evr );
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
        }
      }

      vtmap[oid].add( addr, grp );
      grmap.insert( make_pair(grp,oid) );
      // cerr << "**** " << grp << " " << xmt::getpid() << endl;
    }
  }
}

void Janus::VSNewRemoteMemberRevert( const stem::Event_base<VSsync_rq>& ev )
{
  if ( ev.value().grp != vshosts_group ) {
    group_type grp = ev.value().grp;
    pair<gid_map_type::const_iterator,gid_map_type::const_iterator> range = grmap.equal_range( grp );
    if ( range.first != range.second ) { // we have local? member within this group
      gaddr_type oid = manager()->reflect( ev.src() ); // ???? oid == gaddr
      addr_type addr = ev.src();
      for ( ; range.first != range.second; ++range.first ) {
        vt_map_type::iterator i = vtmap.find( range.first->second );
        if ( i != vtmap.end() && ((i->second.stem_addr() & stem::extbit) == 0 )) {
          stem::Event_base<VSsync_rq> evs( VS_NEW_MEMBER );
          evs.dest( i->second.stem_addr() );
          evs.src( addr );
          evs.value().grp = grp;
          Forward( evs );
#ifdef __FIT_VS_TRACE
          try {
            scoped_lock lk(_lock_tr);
            if ( _trs != 0 && _trs->good() && (_trflags & tracegroup) ) {
              *_trs << " -> VS_NEW_MEMBER (remote revert) G" << grp << " "
                    << hex << showbase
                    << evs.src() << " -> " << evs.dest() << dec << endl;
            }
          }
          catch ( ... ) {
          }
#endif // __FIT_VS_TRACE
        }
      }

      vtmap[oid].add( addr, grp );
      grmap.insert( make_pair(grp,oid) );
      // cerr << "**** " << grp << " " << xmt::getpid() << endl;
    }
  } else {
    gaddr_type oid = manager()->reflect( ev.src() ); // ???? oid == gaddr
    addr_type addr = ev.src();

    vtmap[oid].add( addr, vshosts_group );
    grmap.insert( make_pair(static_cast<group_type>(vshosts_group),oid) );
    // cerr << "**** " << vshosts_group << " " << xmt::getpid() << endl;
  }
}

void Janus::connect( const char *host, int port )
{
  _hostmgr->connect( host, port );
}

void Janus::serve( int port )
{
  _hostmgr->serve( port );
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

DEFINE_RESPONSE_TABLE( Janus )
  EV_Event_base_T_( ST_NULL, VS_MESS, JaDispatch, VSmess )
  EV_Event_base_T_( ST_NULL, VS_NEW_MEMBER, VSNewMember, VSsync_rq )
  EV_Event_base_T_( ST_NULL, VS_NEW_REMOTE_MEMBER, VSNewRemoteMemberDirect, VSsync_rq )
  EV_Event_base_T_( ST_NULL, VS_NEW_MEMBER_RV, VSNewRemoteMemberRevert, VSsync_rq )
END_RESPONSE_TABLE

} // namespace janus
