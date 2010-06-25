// -*- C++ -*- Time-stamp: <10/06/24 21:34:31 ptr>

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

#define VS_EVENT_TORDER 0x30c
#define VS_ORDER_CONF   0x30d
#define VS_LEADER       0x30e

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

void torder_vs::check_leader()
{
  if ( !check_remotes() && vt.vt.find( leader_ ) == vt.vt.end() ) {
    next_leader_election();
  }
}

int torder_vs::vs_torder( const stem::Event& inc_ev )
{
  stem::Event_base<vs_event_total_order> ev( VS_EVENT_TORDER );
  stem::addr_type me = self_id();

  ev.value().ev = inc_ev;
  ev.value().id = xmt::uid();
  ev.value().ev.src( me );
  ev.value().ev.dest( me );

  int ret = basic_vs::vs( ev );

  check_leader();

  return ret;
}

void torder_vs::vs_pub_join()
{
  if ( vs_group_size() == 1 ) {
    leader_ = self_id();
    is_leader_ = true;
  }
}

void torder_vs::vs_pub_view_update()
{
  misc::use_syslog<LOG_DEBUG,LOG_USER>() << HERE << ' ' << self_id() << endl;
  if ( is_leader_ ) {
    if ( vs_group_size() > 1 ) {
      misc::use_syslog<LOG_DEBUG,LOG_USER>() << HERE << ' ' << self_id() << endl;
      EventVoid ev( VS_LEADER );
      send_to_vsg( ev );
    }
    return;
  }

  if ( leader_ == stem::badaddr ) {
    return;
  }

#if 0
  next_leader_election();
#endif
}

void torder_vs::vs_leader( const stem::EventVoid& ev )
{
  if ( vs_group_size() > 1 ) {
    vtime::vtime_type::const_iterator i;
    for ( i = vt.vt.begin(); i != vt.vt.end(); ++i ) {
      if ( i->first == ev.src() ) {
        leader_ = ev.src();
        is_leader_ = false;
        break;
      }
    }
  }
}

void torder_vs::vs_process_torder( const stem::Event_base<vs_event_total_order>& ev )
{
  ev.value().ev.src( ev.src() );
  ev.value().ev.dest( ev.dest() );
  ev.value().ev.setf( stem::__Event_Base::vs );

  check_leader();

  // confirmation event
  if ( ev.value().id == xmt::nil_uuid ) {
    misc::use_syslog<LOG_DEBUG,LOG_USER>() << HERE << ' ' << self_id() << ' ' << ev.value().ev.value() << endl;
    // expected VS_ORDER_CONF and non-empty ev.value().conform here 
    for ( std::list<vs_event_total_order::id_type>::const_iterator i = ev.value().conform.begin();
          i != ev.value().conform.end(); ++i ) {
      conf_cnt_type::iterator k = conform_container_.find( *i );
      if ( k != conform_container_.end() ) {
        misc::use_syslog<LOG_DEBUG,LOG_USER>() << HERE << ' ' << self_id() << ' ' << k->second.value() << endl;
        k->second.setf( stem::__Event_Base::vs );
        this->vs_pub_tord_rec( k->second );
        torder_vs::sync_call( k->second );
        conform_container_.erase( k );
        orig_order_cnt_type::iterator j = find( orig_order_container_.begin(), orig_order_container_.end(), *i );
        if ( j != orig_order_container_.end() ) {
          orig_order_container_.erase( j );
        }
      }
    }
  } else {
    if ( ev.value().conform.empty() ) {
      misc::use_syslog<LOG_DEBUG,LOG_USER>() << HERE << ' ' << self_id() << ' ' << ev.value().ev.value() << endl;
      conform_container_[ev.value().id] = ev.value().ev;
      orig_order_container_.push_back( ev.value().id );
    }

    if ( is_leader() ) { 
      misc::use_syslog<LOG_DEBUG,LOG_USER>() << HERE << ' ' << self_id() << ' ' << ev.value().ev.value() << endl;
      stem::Event_base<vs_event_total_order> cnf( VS_EVENT_TORDER );

      cnf.value().ev.code( VS_ORDER_CONF );
      cnf.value().id = xmt::nil_uuid;
      cnf.value().conform.push_back( ev.value().id );
      vs( cnf );
    }
  }
}

void torder_vs::next_leader_election()
{
  if ( leader_ == badaddr || vs_group_size() < 2 ) {
    return;
  }

  vector<stem::addr_type> basket( vs_group_size() );

  int j = 0;
  for ( vtime::vtime_type::iterator i = vt.vt.begin(); i != vt.vt.end(); ++i, ++j ) {
    basket[j] = i->first;
  }

  sort( basket.begin(), basket.end() );

  vector< stem::addr_type >::iterator i = lower_bound( basket.begin(), basket.end(), leader_ );
  if ( *i == leader_ ) {
    ++i;
    if ( i == basket.end() ) {
      i = basket.begin();
    }
  }

  stem::addr_type sid = self_id();

  if ( *i == sid ) {
    is_leader_ = true;
    leader_ = sid;
    vs_send_flush();
    
    stem::Event_base<vs_event_total_order> cnf( VS_EVENT_TORDER );
    cnf.value().ev.code( VS_ORDER_CONF );
    cnf.value().id = xmt::nil_uuid;
    copy(orig_order_container_.begin(), orig_order_container_.end(), back_inserter(cnf.value().conform) );
    vs( cnf );
  }
}

DEFINE_RESPONSE_TABLE( torder_vs )
  EV_Event_base_T_( ST_NULL, VS_EVENT_TORDER, vs_process_torder, vs_event_total_order )
  EV_Event_base_T_( ST_NULL, VS_LEADER, vs_leader, void )
END_RESPONSE_TABLE

} // namespace janus
