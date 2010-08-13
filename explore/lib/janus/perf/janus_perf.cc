// -*- C++ -*- Time-stamp: <09/08/03 15:58:23 ptr>

/*
 *
 * Copyright (c) 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "janus_perf.h"
#include <exam/suite.h>

#include <sockios/sockstream>
#include <sockios/sockmgr.h>

#include <mt/shm.h>
#include <sys/wait.h>
#include <signal.h>
#include <algorithm>

#include <mt/mutex>
#include <mt/condition_variable>
#include <mt/date_time>

#include <stem/NetTransport.h>

using namespace std;
using namespace std::tr2;
using namespace stem;



VT_with_leader_recovery::VT_with_leader_recovery( const char* nm ) :
    torder_vs(),
    msg_status( *this ),
    gs_status( *this ),
    flush_status( *this ),
    msg(0),
    flush(0)
{
  history.open( nm, ios_base::in | ios_base::out /* | ios_base::app */ );
  if ( !history.is_open() ) {
    history.clear();
    history.open( nm, ios_base::in | ios_base::out | ios_base::trunc );
    uint64_t last_flush_off = 0;
    stem::__pack_base::__pack( history, last_flush_off );
  }

  enable();
}

VT_with_leader_recovery::~VT_with_leader_recovery()
{
  disable();
}

bool VT_with_leader_recovery::_msg_status::operator()() const
{
  return me.msg == me.n_msg;
}

bool VT_with_leader_recovery::_gs_status::operator()() const
{
  return me.vs_group_size() == me.gsize;
}

bool VT_with_leader_recovery::_flush_status::operator()() const
{
  return me.flush >= me.n_flush;
}

xmt::uuid_type VT_with_leader_recovery::vs_pub_recover()
{
  stem::Event ev;
  stem::code_type c;
  uint32_t f;
  xmt::uuid_type flush_id = xmt::nil_uuid;

  ev.dest( self_id() );
  ev.src( stem::badaddr );

  try {
    // Try to read serialized events and re-play history.
    // Note: don't call this function from ctor,
    // because
    //   - it _virtual_ function
    //   - it call 'replay' that call StEM's Dispatch,
    //     and Dispatch is virtual too, and processing
    //     depend upon data from this class.
    history.seekg( 0, ios_base::beg );
    uint64_t last_flush_off = 0;

    stem::__pack_base::__unpack( history, last_flush_off );
    if ( !history.fail() ) { // offset for last flush ok
      while ( !history.fail() ) {
        stem::__pack_base::__unpack( history, c );
        ev.code( c );
        stem::__pack_base::__unpack( history, f );
        ev.resetf( f );
        stem::__pack_base::__unpack( history, ev.value() );

        if ( !history.fail() ) {
          if ( history.tellg() <= last_flush_off ) {
            if ( ev.code() == VS_FLUSH_RQ ) {
              stem::Event_base<xmt::uuid_type> fev;
              fev.unpack( ev );              
              flush_id = fev.value();
            } else {
              // basic_vs::sync_call( ev );
              this->Dispatch( ev );
            }
          } else {
            break;
          }
        }
      }
      history.clear();
      history.seekp( max( last_flush_off, static_cast<uint64_t>(sizeof(last_flush_off)) ), ios_base::beg );
    } else { // can't recover offset after last flush
      history.clear();
      history.seekp( 0, ios_base::beg );
      last_flush_off = 0;
      stem::__pack_base::__pack( history, last_flush_off );
      flush_id = xmt::nil_uuid;
    }
  }
  catch ( const std::runtime_error& err ) {
    history.clear();
    history.seekp( 0, ios_base::end );
    flush_id = xmt::nil_uuid;
  }

  return flush_id;
}

void VT_with_leader_recovery::vs_resend_from( const xmt::uuid_type& from, const stem::addr_type& addr)
{
  if ( !is_avail( addr ) ) {
    return;
  }

  stem::Event ev;
  stem::code_type c;
  uint32_t f;
  bool ref_point_found = (from == xmt::nil_uuid) ? true : false;


  ev.dest( addr );
  ev.src( self_id() );

  history.seekg( sizeof(uint64_t), ios_base::beg );

  while ( !history.fail() ) {
    stem::__pack_base::__unpack( history, c );
    ev.code( c );
    stem::__pack_base::__unpack( history, f );
    ev.resetf( f );
    stem::__pack_base::__unpack( history, ev.value() );

    if ( !history.fail() ) {
      if ( !ref_point_found ) {
        if ( ev.code() == VS_FLUSH_RQ ) {
          stem::Event_base<xmt::uuid_type> fev;
          fev.unpack( ev );              
          if ( fev.value() == from ) {
            ref_point_found = true;
          }
        }
      } else {
        /* flag stem::__Event_Base::vs is set to avoid forwarding
           'recovery' event to other group members;
           stem::__Event_Base::vs_join is set to distinguish event
           as come during recovery and record it (see VTM_one_group_recover::message,
           and call VTM_one_group_recover::vs_event_derivative in
           VTM_one_group_recover::message).
         */
        ev.setf( stem::__Event_Base::vs | stem::__Event_Base::vs_join );
        // Change event, to avoid interference with
        // true VS_FLUSH_VIEW---it work with view lock,
        // but in this case I want to bypass locking
        if ( ev.code() != VS_FLUSH_RQ ) {
          Forward( ev ); // src was set above
        }
      }
    }
  }

  history.clear();
  torder_vs::vs_resend_from( from, addr );
}

void VT_with_leader_recovery::vs_pub_view_update()
{
  torder_vs::vs_pub_view_update();
  std::tr2::lock_guard<std::tr2::mutex> lk( mtx );
  cnd.notify_one();
}

void VT_with_leader_recovery::vs_pub_rec( const stem::Event& ev )
{
  if ( ev.code() == VS_FLUSH_RQ ) {
    stem::__pack_base::__pack( history, ev.code() );
    stem::__pack_base::__pack( history, ev.flags() );
    stem::__pack_base::__pack( history, ev.value() );
  }
}

void VT_with_leader_recovery::vs_pub_flush()
{
  torder_vs::vs_pub_flush();
  uint64_t last_flush_off = history.tellp();
  history.seekp( 0, ios_base::beg );
  stem::__pack_base::__pack( history, last_flush_off );

  history.seekp( 0, ios_base::end );
  std::tr2::lock_guard<std::tr2::mutex> lk( mtx );
  ++flush;
  cnd.notify_one();
}

void VT_with_leader_recovery::vs_pub_tord_rec( const stem::Event& ev )
{
  stem::__pack_base::__pack( history, ev.code() );
  stem::__pack_base::__pack( history, ev.flags() );
  stem::__pack_base::__pack( history, ev.value() );
}

std::tr2::milliseconds VT_with_leader_recovery::vs_pub_lock_timeout() const {
  return std::tr2::milliseconds(250);
}

void VT_with_leader_recovery::message( const stem::Event& ev )
{
  // retranslate this message within virtual synchrony group
  // with total order of events

  stem::Event sync( EV_VS_EV_SAMPLE );

  sync.value() = ev.value();

  vs_torder( sync );
}

void VT_with_leader_recovery::sync_message( const stem::Event& ev )
{
  if ( ev.flags() & stem::__Event_Base::vs_join ) {
    vs_pub_tord_rec( ev );
  }
  std::tr2::lock_guard<std::tr2::mutex> lk( mtx );
  ++msg;
  cnd.notify_one();
}

DEFINE_RESPONSE_TABLE( VT_with_leader_recovery )
  EV_EDS( ST_NULL, EV_EXT_EV_SAMPLE, message )
  EV_EDS( ST_NULL, EV_VS_EV_SAMPLE, sync_message )
END_RESPONSE_TABLE

int janus_perf::n_obj;
int janus_perf::n_msg;
std::vector< std::tr2::thread* > janus_perf::thr;
std::vector< std::string > janus_perf::names;
std::vector< int > janus_perf::res;
stem::addr_type janus_perf::addr;

static void run(int i);
void janus_perf::run(int i)
{
  VT_with_leader_recovery a( names[i].c_str() );
  a.vs_join( addr );

  EXAM_CHECK_ASYNC( a.wait_group_size( std::tr2::milliseconds(n_obj * 200), n_obj + 1 ) );

  stem::Event ev( EV_EXT_EV_SAMPLE );
  ev.dest( a.self_id() );

  for ( int j = 0; j < n_msg; ++j ) {
    std::stringstream v;
    v << j;
    ev.value() = v.str();
    a.Send( ev );
  }

  a.vs_send_flush();

  EXAM_CHECK_ASYNC_F( a.wait_msg( std::tr2::milliseconds(n_msg * n_obj * 100), n_obj * n_msg ), res[i] );
  EXAM_CHECK_ASYNC_F( a.wait_flush( std::tr2::milliseconds(n_obj * 200), n_obj ), res[i] );
}

