// -*- C++ -*- Time-stamp: <10/01/27 20:27:43 ptr>

/*
 *
 * Copyright (c) 2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include <janus/torder.h>

namespace janus {

using namespace std;
using namespace xmt;
using namespace stem;
using namespace std::tr2;

void vs_event_total_order::pack( std::ostream& s ) const
{
  // basic_event::pack( s );
  __pack( s, id );
  __pack( s, static_cast<uint32_t>(conform.size()) );
  for ( list<uuid_type>::const_iterator i = conform.begin(); i != conform.end(); ++i ) {
    __pack( s, *i );
  }
  __pack( s, ev.code() );
  __pack( s, ev.flags() );
  __pack( s, ev.value() );
}

void vs_event_total_order::unpack( std::istream& s )
{
  // basic_event::unpack( s );
  __unpack( s, id );
  uint32_t sz = 0;
  uuid_type tmp;
  __unpack( s, sz );
  while ( sz > 0 ) {
    __unpack( s, tmp );
    conform.push_back( tmp );
    --sz;
  }

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

void vs_event_total_order::swap( vs_event_total_order& r )
{
  // std::swap( vt, r.vt );
  std::swap( id, r.id );
  std::swap( conform, r.conform );
  std::swap( ev, r.ev );
}

torder_vs::torder_vs() :
    basic_vs(),
    leader_( stem::badaddr ),
    is_leader_( false )
{
}

torder_vs::torder_vs( const char* info ) :
    basic_vs( info ),
    leader_( stem::badaddr ),
    is_leader_( false )
{
}

torder_vs::~torder_vs()
{
}

void torder_vs::leader()
{
  leader_ = self_id();
  is_leader_ = true;
}

int torder_vs::vs_torder( const stem::Event& inc_ev )
{
  stem::Event_base<vs_event_total_order> ev( VS_EVENT_TORDER );

  ev.value().ev = inc_ev;
  ev.value().id = xmt::uid();

  if ( is_leader() ) {
    ev.value().conform.push_back( ev.value().id );
  } else {
    conform_container_[ev.value().id] = ev.value().ev;
  }

  int ret = basic_vs::vs_aux( ev );

  if ( is_leader() ) {
    torder_vs::sync_call( inc_ev );
  }

  return ret;
}

int torder_vs::vs_torder_aux( const stem::Event& inc_ev )
{
  stem::Event_base<vs_event_total_order> ev( VS_EVENT_TORDER );

  ev.value().ev = inc_ev;
  ev.value().id = xmt::uid();

  if ( is_leader() ) {
    ev.value().conform.push_back( ev.value().id );
  } else {
    conform_container_[ev.value().id] = ev.value().ev;
  }

  // don't forget process event on this node!
  return basic_vs::vs_aux( ev );
}

void torder_vs::vs_pub_join()
{
  if ( vs_group_size() == 1 ) {
    leader_ = self_id();
    is_leader_ = true;
  }
}

// void torder_vs::vs_send_flush()
// {
// }

void torder_vs::vs_process_torder( const stem::Event_base<vs_event_total_order>& ev )
{
  ev.value().ev.src( ev.src() );
  ev.value().ev.dest( ev.dest() );
  ev.value().ev.setf( stem::__Event_Base::vs );

  if ( is_leader() ) {
    // I'm leader; original event not from me;
    // send conformation first ...
    stem::Event_base<vs_event_total_order> cnf( VS_EVENT_TORDER );

    cnf.value().ev.code( VS_ORDER_CONF );
    cnf.value().id = xmt::nil_uuid;
    cnf.value().conform.push_back( ev.value().id );
    vs_aux( cnf );

    // ... and then process
    torder_vs::sync_call( ev.value().ev );
  } else {
    if ( ev.value().id == xmt::nil_uuid ) {
      // expected VS_ORDER_CONF and non-empty ev.value().conform here
      for ( std::list<vs_event_total_order::id_type>::const_iterator i = ev.value().conform.begin();
            i != ev.value().conform.end(); ++i ) {
        conf_cnt_type::iterator k = conform_container_.find( *i );
        if ( k != conform_container_.end() ) {
          k->second.setf( stem::__Event_Base::vs );
          torder_vs::sync_call( k->second );
          conform_container_.erase( k );
        } // else {
          /* I see conformation, but no event [yet]?
             Because it _after_ casual order processing,
             this shouldn't happens.
           */
          //   cerr << HERE << endl;
        // }
      }
    } else if ( ev.value().conform.empty() ) {
      // I'm not a leader, no conformations in event
      conform_container_[ev.value().id] = ev.value().ev;
    } else if ( find( ev.value().conform.begin(), ev.value().conform.end(), ev.value().id ) != ev.value().conform.end() ) {
      // event origin: leader; I see conformation in event
      torder_vs::sync_call( ev.value().ev );
    } else {
      // it's from me?
      for ( std::list<vs_event_total_order::id_type>::const_iterator i = ev.value().conform.begin();
            i != ev.value().conform.end(); ++i ) {
        conf_cnt_type::iterator k = conform_container_.find( *i );
        if ( k != conform_container_.end() ) {
          k->second.setf( stem::__Event_Base::vs );
          torder_vs::sync_call( k->second );
          conform_container_.erase( k );
        } // else {
          /* I see conformation, but no event [yet]?
             Because it _after_ casual order processing,
             this shouldn't happens.
           */
        //  cerr << HERE << ' ' << ev.value().id << endl;
        //  conform_container_[ev.value().id] = ev.value().ev;
        // }
      }
    }
  }
}

const stem::code_type torder_vs::VS_EVENT_TORDER = 0x301;
const stem::code_type torder_vs::VS_ORDER_CONF = 0x303;

DEFINE_RESPONSE_TABLE( torder_vs )
  EV_Event_base_T_( ST_NULL, VS_EVENT_TORDER, vs_process_torder, vs_event_total_order )
END_RESPONSE_TABLE

} // namespace janus
