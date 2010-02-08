// -*- C++ -*- Time-stamp: <10/02/08 17:17:19 ptr>

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

int torder_vs::vs_torder( const stem::Event& inc_ev )
{
  stem::Event_base<vs_event_total_order> ev( VS_EVENT_TORDER );
  stem::addr_type me = self_id();

  ev.value().ev = inc_ev;
  ev.value().id = xmt::uid();
  ev.value().ev.src( me );
  ev.value().ev.dest( me );

  if ( is_leader() ) {
    ev.value().conform.push_back( ev.value().id );
  } else {
    conform_container_[ev.value().id] = ev.value().ev;
    orig_order_container_.push_back( ev.value().id );
  }

  int ret = basic_vs::vs_aux( ev );

  if ( is_leader_ ) {
    if ( ret == 0 ) {
      ev.value().ev.setf( stem::__Event_Base::vs );
      this->vs_pub_tord_rec( ev.value().ev );
      torder_vs::sync_call( ev.value().ev );
    }
  } else if ( conform_container_.size() > 0 ) {
    check_remotes();

    vtime::vtime_type::const_iterator i;
    for ( i = vt.vt.begin(); i != vt.vt.end(); ++i ) {
      if ( i->first == leader_ ) {
        break;
      }
    }

    if ( i == vt.vt.end() ) { // leader leave us, and not in vt already
      next_leader_election();
    }
  }

  return ret;
}

int torder_vs::vs_torder_aux( const stem::Event& inc_ev )
{
  stem::Event_base<vs_event_total_order> ev( VS_EVENT_TORDER );
  stem::addr_type me = self_id();

  ev.value().ev = inc_ev;
  ev.value().id = xmt::uid();
  ev.value().ev.src( me );
  ev.value().ev.dest( me );

  if ( is_leader() ) {
    ev.value().conform.push_back( ev.value().id );
  } else {
    conform_container_[ev.value().id] = ev.value().ev;
    orig_order_container_.push_back( ev.value().id );
  }

  // don't forget process event on this node!
  int ret = basic_vs::vs_aux( ev );

  if ( !is_leader_ && (conform_container_.size() > 3) ) {
    check_remotes();

    vtime::vtime_type::const_iterator i;
    for ( i = vt.vt.begin(); i != vt.vt.end(); ++i ) {
      if ( i->first == leader_ ) {
        break;
      }
    }
    if ( i == vt.vt.end() ) {
      next_leader_election();
    }
  }

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
  if ( is_leader_ ) {
    if ( vs_group_size() > 1 ) {
      EventVoid ev( VS_LEADER );
      send_to_vsg( ev );
    }
    return;
  }

  if ( leader_ == stem::badaddr ) {
    return;
  }

  next_leader_election();
}

void torder_vs::vs_leader( const stem::EventVoid& ev )
{
  // if ( is_leader_ ) {
  //   cerr << HERE << endl;
  // }

  if ( vs_group_size() > 1 ) {
    vtime::vtime_type::const_iterator i;
    for ( i = vt.vt.begin(); i != vt.vt.end(); ++i ) {
      if ( i->first == ev.src() ) {
        leader_ = ev.src();
        is_leader_ = false;
        break;
      }
    }

    // if ( i == vt.vt.end() ) {
      // who is here? who is event author?
    //   cerr << HERE << endl;
    // }

    // leader_ = ev.src();
    // is_leader_ = false;
  } else {
    // become leader...?
    // cerr << HERE << endl;
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
    ev.value().ev.setf( stem::__Event_Base::vs );
    this->vs_pub_tord_rec( ev.value().ev );
    torder_vs::sync_call( ev.value().ev );
  } else {
    if ( ev.value().id == xmt::nil_uuid ) {
      // expected VS_ORDER_CONF and non-empty ev.value().conform here
      for ( std::list<vs_event_total_order::id_type>::const_iterator i = ev.value().conform.begin();
            i != ev.value().conform.end(); ++i ) {
        conf_cnt_type::iterator k = conform_container_.find( *i );
        if ( k != conform_container_.end() ) {
          k->second.setf( stem::__Event_Base::vs );
          // this->vs_pub_tord_rec( ev.value().ev ); // conformation
          torder_vs::sync_call( k->second );
          conform_container_.erase( k );
          orig_order_cnt_type::iterator j = find( orig_order_container_.begin(), orig_order_container_.end(), *i );
          if ( j != orig_order_container_.end() ) {
            orig_order_container_.erase( j );
          }
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
      orig_order_container_.push_back( ev.value().id );
    } else if ( find( ev.value().conform.begin(), ev.value().conform.end(), ev.value().id ) != ev.value().conform.end() ) {
      // event origin: leader; I see conformation in event
      ev.value().ev.setf( stem::__Event_Base::vs );
      this->vs_pub_tord_rec( ev.value().ev );
      torder_vs::sync_call( ev.value().ev );
    } else {
      // it's from me?
      for ( std::list<vs_event_total_order::id_type>::const_iterator i = ev.value().conform.begin();
            i != ev.value().conform.end(); ++i ) {
        conf_cnt_type::iterator k = conform_container_.find( *i );
        if ( k != conform_container_.end() ) {
          k->second.setf( stem::__Event_Base::vs );
          this->vs_pub_tord_rec( ev.value().ev );
          torder_vs::sync_call( k->second );
          conform_container_.erase( k );
          orig_order_cnt_type::iterator j = find( orig_order_container_.begin(), orig_order_container_.end(), *i );
          if ( j != orig_order_container_.end() ) {
            orig_order_container_.erase( j );
          }
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

void torder_vs::next_leader_election()
{
  // requirements here: leader_ != badaddr

  vector<stem::addr_type> basket;

  basket.reserve( vs_group_size() + 1 );

  for ( vtime::vtime_type::const_iterator i = vt.vt.begin(); i != vt.vt.end(); ++i ) {
    if ( i->first != leader_ ) {
      basket.push_back( i->first );
    }
  }

  if ( basket.size() == vs_group_size() ) { // leader leave us
    basket.push_back( leader_ );
    sort( basket.begin(), basket.end() );
    for ( vector<stem::addr_type>::const_iterator i = basket.begin(); i != basket.end(); ++i ) {
      if ( *i == leader_ ) {
        ++i;
        if ( i == basket.end() ) {
          i = basket.begin();
        }
        leader_ = *i;
        if ( *i == self_id() ) {
          is_leader_ = true;
          stem::Event_base<vs_event_total_order> cnf( VS_EVENT_TORDER );
          cnf.value().ev.code( VS_ORDER_CONF );
          cnf.value().id = xmt::nil_uuid;

          // I'm leader now;
          // send conformation first ...
          copy(orig_order_container_.begin(), orig_order_container_.end(), back_inserter(cnf.value().conform) );
          vs_aux( cnf );

          conf_cnt_type::iterator k;

          // ... and then process
          for ( orig_order_cnt_type::iterator j = orig_order_container_.begin(); j != orig_order_container_.end(); ) {
            k = conform_container_.find( *j );
            if ( k != conform_container_.end() ) {
              k->second.setf( stem::__Event_Base::vs );

              this->vs_pub_tord_rec( k->second );
              torder_vs::sync_call( k->second );
              conform_container_.erase( k );
            }
            orig_order_container_.erase( j++ );
          }
          // EventVoid ev( VS_LEADER );
          // send_to_vsg( ev );
        }
        break;
      }
    }
  }
}

const stem::code_type torder_vs::VS_EVENT_TORDER = 0x301;
const stem::code_type torder_vs::VS_ORDER_CONF   = 0x303;
const stem::code_type torder_vs::VS_LEADER       = 0x300;

DEFINE_RESPONSE_TABLE( torder_vs )
  EV_Event_base_T_( ST_NULL, VS_EVENT_TORDER, vs_process_torder, vs_event_total_order )
  EV_Event_base_T_( ST_NULL, VS_LEADER, vs_leader, void )
END_RESPONSE_TABLE

} // namespace janus
