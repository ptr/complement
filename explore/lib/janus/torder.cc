// -*- C++ -*- Time-stamp: <10/07/12 12:32:57 ptr>

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

#define VS_EVENT_TORDER 0x30d
#define VS_ORDER_CONF   0x30e

torder_vs::torder_vs() :
    basic_vs(),
    is_leader_( false )
{
}

torder_vs::torder_vs( const char* info ) :
    basic_vs( info ),
    is_leader_( false )
{
}

int torder_vs::vs_torder( const stem::Event& inc_ev )
{
  stem::Event_base<vs_event_total_order> ev( VS_EVENT_TORDER );
  stem::addr_type sid = self_id();

  ev.value().ev = inc_ev;
  ev.value().id = xmt::uid();
  ev.value().ev.src( sid );
  ev.value().ev.dest( sid );

  int ret = basic_vs::vs( ev );

  return ret;
}

void torder_vs::vs_pub_flush()
{
  torder_vs::vs_pub_view_update();
}

void torder_vs::vs_pub_view_update()
{
  stem::addr_type sid = self_id();
  if ( sid == badaddr ) {
    return;
  }

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

  if ( *i == sid ) {
    is_leader_ = true;
      
    stem::Event_base<vs_event_total_order::id_type> cnf( VS_ORDER_CONF );
    orig_order_cnt_type tmp( orig_order_container_.begin(), orig_order_container_.end() );
    for ( orig_order_cnt_type::iterator j = tmp.begin(); j != tmp.end(); ++j) {
      misc::use_syslog<LOG_INFO,LOG_USER>() << HERE << ':' << sid << ':' << *j << endl;
      cnf.value() = *j;
      vs( cnf );
    }
  } else {
    is_leader_ = false;
  }
}

void torder_vs::vs_process_torder( const stem::Event_base<vs_event_total_order>& ev )
{
  ev.value().ev.src( ev.src() );
  ev.value().ev.dest( ev.dest() );
  ev.value().ev.setf( stem::__Event_Base::vs );

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
    // torder_vs::sync_call( k->second );
    this->Dispatch( k->second );
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
END_RESPONSE_TABLE

} // namespace janus
