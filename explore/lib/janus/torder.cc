// -*- C++ -*- Time-stamp: <10/07/02 13:03:29 ptr>

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
  check_remotes();

  bool leader_leave;
  {
    unique_lock<recursive_mutex> lk( _lock_vt );
    leader_leave = ( vt.vt.find( leader_ ) == vt.vt.end() );
  }

  if ( leader_ != stem::badaddr && leader_leave) {
    // next leader election process
    vector<stem::addr_type> basket;
    {
      unique_lock<recursive_mutex> lk( _lock_vt );
      basket.resize( vt.vt.size() );
      int j = 0;
      for ( vtime::vtime_type::iterator i = vt.vt.begin(); i != vt.vt.end(); ++i, ++j ) {
        basket[j] = i->first;
      }
    }

    sort( basket.begin(), basket.end() );

    vector<stem::addr_type>::iterator i = basket.begin() + view % basket.size();

    stem::addr_type sid = self_id();

    leader_ = *i;

    if ( *i == sid ) {
      is_leader_ = true;
      vs_send_flush();
        
      stem::Event_base<vs_event_total_order::id_type> cnf( VS_ORDER_CONF );
      orig_order_cnt_type tmp( orig_order_container_.begin(), orig_order_container_.end() );
      for ( orig_order_cnt_type::iterator j = tmp.begin(); j != tmp.end(); ++j) {
        cnf.value() = *j;
        vs( cnf );
      }
    }
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
  if ( is_leader() ) {
    if ( vs_group_size() > 1 ) {
      EventVoid ev( VS_LEADER );
      send_to_vsg( ev );
    }
  }
}

void torder_vs::vs_leader( const stem::EventVoid& ev )
{
  {
    lock_guard<recursive_mutex> lk( _lock_vt );
    if ( vt.vt.size() > 1 ) {
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
}

void torder_vs::vs_process_torder( const stem::Event_base<vs_event_total_order>& ev )
{
  ev.value().ev.src( ev.src() );
  ev.value().ev.dest( ev.dest() );
  ev.value().ev.setf( stem::__Event_Base::vs );

  check_leader();

  conform_container_[ev.value().id] = ev.value().ev;
  orig_order_container_.push_back( ev.value().id );

  if ( is_leader() ) {
    stem::Event_base<vs_event_total_order::id_type> cnf( VS_ORDER_CONF );
    cnf.value() = ev.value().id;
    vs( cnf );
  }
}

void torder_vs::vs_torder_conf( const stem::Event_base<vs_event_total_order::id_type>& ev )
{
  conf_cnt_type::iterator k = conform_container_.find( ev.value() );
  if ( k != conform_container_.end() ) {
    this->vs_pub_tord_rec( k->second );
    torder_vs::sync_call( k->second );
    conform_container_.erase( k );
    orig_order_cnt_type::iterator j = find( orig_order_container_.begin(), orig_order_container_.end(), ev.value() );
    if ( j != orig_order_container_.end() ) {
      orig_order_container_.erase( j );
    }
  } else {
    misc::use_syslog<LOG_INFO,LOG_USER>() << HERE << ':' << self_id() << ':' << ev.value() << ':' << "unexpected" << endl;
  }
}

DEFINE_RESPONSE_TABLE( torder_vs )
  EV_Event_base_T_( ST_NULL, VS_EVENT_TORDER, vs_process_torder, vs_event_total_order )
  EV_Event_base_T_( ST_NULL, VS_ORDER_CONF, vs_torder_conf, vs_event_total_order::id_type )
  EV_Event_base_T_( ST_NULL, VS_LEADER, vs_leader, void )
END_RESPONSE_TABLE

} // namespace janus
