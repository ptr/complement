// -*- C++ -*- Time-stamp: <10/06/10 22:29:15 ptr>

/*
 *
 * Copyright (c) 2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "vt_operations.h"
#include "leader_recovery.h"

#include <iostream>
#include <janus/vtime.h>

#include <mt/mutex>
#include <mt/condition_variable>
#include <mt/date_time>
#include <sys/wait.h>
#include <mt/shm.h>
#include <mt/thread>
#include <sockios/sockmgr.h>
#include <stem/NetTransport.h>

#include <algorithm>
#include <set>
#include <list>

#include <fstream>
#include <mt/uid.h>
#include <unistd.h>

namespace janus {

using namespace std;

#define EV_EXT_EV_SAMPLE      0x9010
#define EV_VS_EV_SAMPLE       0x9011
#define EV_VS_EV_SAMPLE2      0x9012

#ifndef VS_FLUSH_RQ
# define VS_FLUSH_RQ 0x307 // see casual.cc
#endif

#ifndef VS_EVENT_TORDER
# define VS_EVENT_TORDER 0x30d // see torder.cc
#endif

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
  return me.flush == me.n_flush;
}

xmt::uuid_type VT_with_leader_recovery::vs_pub_recover()
{
  stem::Event ev;
  stem::code_type c;
  uint32_t f;
  xmt::uuid_type flush_id = xmt::nil_uuid;

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
              basic_vs::sync_call( ev );
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

  if ( !conform_container_.empty() ) {
    for ( conf_cnt_type::const_iterator i = conform_container_.begin();i != conform_container_.end();++i) {
      stem::Event_base<vs_event_total_order> ev( VS_EVENT_TORDER );
      ev.value().ev = i->second;
      ev.value().id = i->first;
      ev.src( i->second.src() );
      ev.dest( addr );
      Forward( ev );
    }
  }

  history.clear();
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

int EXAM_IMPL(vtime_operations::leader_multiple_change)
{
  const int n = 10;
  stem::Event ev( EV_EXT_EV_SAMPLE );

  vector< string > names(n);
  vector<VT_with_leader_recovery*> a(n);

  for ( int i = 0; i < n; ++i ) {
    names[i] = string("/tmp/janus.") + xmt::uid_str();
    a[i] = new VT_with_leader_recovery( names[i].c_str() );
  }

  a[0]->vs_join( stem::badaddr );
  EXAM_CHECK( a[0]->is_leader() );

  int k = 0;
  for ( int i = 1; i < n; ++i ) {
    a[i]->vs_join( a[i - 1]->self_id() );
    for (int j = 0;j < i;++j) {
      stringstream ss;
      ss << i << ' ' << j;
      ev.value() = ss.str();
      ev.dest( a[j]->self_id() );
      a[j]->Send( ev );
    }
    k += i;
    EXAM_CHECK( a[i]->wait_group_size( std::tr2::milliseconds(n * 200), i + 1 ) );
    EXAM_CHECK( a[i]->wait_msg( std::tr2::milliseconds(n * 200), k ) );
  }


  for ( int i = 0; i < n; ++i ) {
    delete a[i];
    for (int j = i + 1;j < n;++j) {
      stringstream ss;
      ss << i << ' ' << j;
      ev.value() = ss.str();
      ev.dest( a[j]->self_id() );
      a[j]->Send( ev );
    }
    k += n - i - 1;
    for (int j = i + 1;j < n;++j) {
      EXAM_CHECK( a[j]->wait_group_size( std::tr2::milliseconds(n * 300), n - i - 1 ) );
      EXAM_CHECK( a[j]->wait_msg( std::tr2::milliseconds(n * 300), k ) );
      if ( EXAM_RESULT ) {
        cout << i << ' ' << j << ' ' << k << ' ' << a[j]->msg << endl;
        goto park;
      }
    }
  }
  
  park:
  ;

  for ( int i = 0; i < n; ++i ) {
    unlink( names[i].c_str() );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::leader_recovery)
{
  const int n_msg1 = 1000;
  const int n_msg2 = 1000;
  const int n_msg3 = 1000;

  try {
    xmt::shm_alloc<0> seg;
    seg.allocate( 70000, 4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    xmt::allocator_shm<std::tr2::barrier_ip,0> shm;
    xmt::allocator_shm<stem::addr_type,0> shm_a;

    std::tr2::barrier_ip& b = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip();
    std::tr2::barrier_ip& b2 = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip( 3 );
    std::tr2::barrier_ip& b3 = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip();
    std::tr2::barrier_ip& b4 = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip( 3 );
    std::tr2::barrier_ip& b5 = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip( 3 );
    std::tr2::barrier_ip& b6 = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip( 3 );
    std::tr2::barrier_ip& b7 = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip( 3 );
    stem::addr_type& addr = *new ( shm_a.allocate( 1 ) ) stem::addr_type();
    stem::addr_type& addr1 = *new ( shm_a.allocate( 1 ) ) stem::addr_type();

    addr = stem::badaddr;
    addr1 = stem::badaddr;

    try {
      tr2::this_thread::fork();

      std::tr2::this_thread::block_signal( SIGINT );
      std::tr2::this_thread::block_signal( SIGQUIT );
      std::tr2::this_thread::block_signal( SIGILL );
      std::tr2::this_thread::block_signal( SIGABRT );
      std::tr2::this_thread::block_signal( SIGFPE );
      std::tr2::this_thread::block_signal( SIGSEGV );
      std::tr2::this_thread::block_signal( SIGTERM );
      std::tr2::this_thread::block_signal( SIGPIPE );

      int res = 0;

      try { // establish group: first member
        VT_with_leader_recovery a1( "/tmp/a1" );

        addr = a1.self_id();

        connect_processor<stem::NetTransport> srv( 2009 );

        EXAM_CHECK_ASYNC_F( srv.is_open(), res );

        list<net_iface> ifaces;

        get_ifaces( back_inserter(ifaces) );

        if ( !ifaces.empty() ) {
          for ( list<net_iface>::iterator f = ifaces.begin(); f != ifaces.end(); ++f ) {
            if ( f->addr.any.sa_family == PF_INET ) {
              f->addr.inet.sin_port = stem::to_net( static_cast<short>(2009) );
              a1.vs_tcp_point( f->addr.inet );
            }
          }
        }

        a1.vs_join( stem::badaddr );

        b.wait();

        EXAM_CHECK_ASYNC_F( a1.wait_group_size( std::tr2::milliseconds(500), 3), res );
        std::tr2::this_thread::sleep( std::tr2::milliseconds(200) );

        b4.wait(); // group size 3

        // a1 is group leader here
        stem::Event ev( EV_EXT_EV_SAMPLE );
        ev.dest( a1.self_id() );

        for ( int j = 0; j < n_msg1; ++j ) {
          stringstream v;
          v << j;
          ev.value() = v.str();
          a1.Send( ev );
        }

        a1.vs_send_flush();

        EXAM_CHECK_ASYNC_F( a1.wait_msg( std::tr2::milliseconds( max( 500, 20 * n_msg1) ), 3 * n_msg1 ), res );
        EXAM_CHECK_ASYNC_F( a1.wait_flush( std::tr2::milliseconds( max( 500, 20 * n_msg1) ), 1 ), res );

        b4.wait(); // group size 3, align with others
      }
      catch ( ... ) {
        EXAM_ERROR_ASYNC_F( "unkown exception", res );
      }

      b2.wait(); // should be here: after dtor of a1

      try { // establish group: first member
        VT_with_leader_recovery a1( "/tmp/a1" );


        connect_processor<stem::NetTransport> srv( 2009 );

        EXAM_CHECK_ASYNC_F( srv.is_open(), res );

        list<net_iface> ifaces;

        get_ifaces( back_inserter(ifaces) );

        if ( !ifaces.empty() ) {
          for ( list<net_iface>::iterator f = ifaces.begin(); f != ifaces.end(); ++f ) {
            if ( f->addr.any.sa_family == PF_INET ) {
              f->addr.inet.sin_port = stem::to_net( static_cast<short>(2009) );
              a1.vs_tcp_point( f->addr.inet );
            }
          }
        }

        a1.vs_join( addr1, "localhost", 2011 );

        b5.wait();

        EXAM_CHECK_ASYNC_F( a1.wait_group_size( std::tr2::milliseconds( max(500, 20 * n_msg2)), 3), res );
        std::tr2::this_thread::sleep( std::tr2::milliseconds(200) );

        b6.wait();

        stem::Event ev( EV_EXT_EV_SAMPLE );
        ev.dest( a1.self_id() );

        for ( int j = 3 * n_msg1 + 2 * n_msg2; j < 3 * n_msg1 + 2 * n_msg2 + n_msg3; ++j ) {
          stringstream v;
          v << j;
          ev.value() = v.str();
          a1.Send( ev );
        }

        EXAM_CHECK_ASYNC_F( a1.wait_msg( std::tr2::milliseconds( max( 500, 20 * n_msg1) ), 3 * n_msg1 + 2 * n_msg2 + 3 * n_msg3 ), res );

        b7.wait(); 
      }
      catch ( ... ) {
        EXAM_ERROR_ASYNC_F( "unkown exception", res );
      }

      exit( res );
    }
    catch ( std::tr2::fork_in_parent& child ) {
      try {
        tr2::this_thread::fork();

        std::tr2::this_thread::block_signal( SIGINT );
        std::tr2::this_thread::block_signal( SIGQUIT );
        std::tr2::this_thread::block_signal( SIGILL );
        std::tr2::this_thread::block_signal( SIGABRT );
        std::tr2::this_thread::block_signal( SIGFPE );
        std::tr2::this_thread::block_signal( SIGSEGV );
        std::tr2::this_thread::block_signal( SIGTERM );
        std::tr2::this_thread::block_signal( SIGPIPE );

        int res = 0;

        b3.wait(); // wait while second member join
        try { // the third member
          VT_with_leader_recovery a3( "/tmp/a3" );

          addr1 = a3.self_id();

          connect_processor<stem::NetTransport> srv( 2011 );

          EXAM_CHECK_ASYNC_F( srv.is_open(), res );

          EXAM_CHECK_ASYNC_F( addr != stem::badaddr, res );

          list<net_iface> ifaces;

          get_ifaces( back_inserter(ifaces) );

          if ( !ifaces.empty() ) {
            for ( list<net_iface>::iterator f = ifaces.begin(); f != ifaces.end(); ++f ) {
              if ( f->addr.any.sa_family == PF_INET ) {
                f->addr.inet.sin_port = stem::to_net( static_cast<short>(2011) );
                a3.vs_tcp_point( f->addr.inet );
              }
            }
          }

          a3.vs_join( addr, "localhost", 2009 );

          EXAM_CHECK_ASYNC_F( a3.wait_group_size( std::tr2::milliseconds(500), 3), res );
          std::tr2::this_thread::sleep( std::tr2::milliseconds(200) );

          b4.wait(); // group size 3

          stem::Event ev( EV_EXT_EV_SAMPLE );
          ev.dest( a3.self_id() );

          for ( int j = 2 * n_msg1; j < 3 * n_msg1; ++j ) {
            stringstream v;
            v << j;
            ev.value() = v.str();
            a3.Send( ev );
          }

          EXAM_CHECK_ASYNC_F( a3.wait_msg( std::tr2::milliseconds( max( 500, 20 * n_msg1) ), 3 * n_msg1 ), res );
          EXAM_CHECK_ASYNC_F( a3.wait_flush( std::tr2::milliseconds( max( 500, 20 * n_msg1) ), 1 ), res );

          b4.wait(); // group size 3, first 10 events with a1 group leader
          std::tr2::this_thread::sleep( std::tr2::milliseconds(120) );

          // a1 go away, leader failure should be detected and new leader
          // should be elected
          for ( int j = 3 * n_msg1; j < 3 * n_msg1 + n_msg2; ++j ) {
            stringstream v;
            v << j;
            ev.value() = v.str();
            a3.Send( ev );
          }

          EXAM_CHECK_ASYNC_F( a3.wait_msg( std::tr2::milliseconds( max( 500, 20 * n_msg2 ) ), 3 * n_msg1 + 2 * n_msg2), res );

          b2.wait();

          b5.wait();

          EXAM_CHECK_ASYNC_F( a3.wait_group_size( std::tr2::milliseconds(500), 3), res );
          std::tr2::this_thread::sleep( std::tr2::milliseconds(200) );
          
          b6.wait();

          for ( int j = 3 * n_msg1 + 2 * n_msg2 + n_msg3; j < 3 * n_msg1 + 2 * n_msg2 + 2 * n_msg3; ++j ) {
            stringstream v;
            v << j;
            ev.value() = v.str();
            a3.Send( ev );
          }

          EXAM_CHECK_ASYNC_F( a3.wait_msg( std::tr2::milliseconds( max( 500, 20 * n_msg2 ) ), 3 * n_msg1 + 2 * n_msg2 + 3 * n_msg3), res );

          b7.wait();
        }
        catch ( ... ) {
          EXAM_ERROR_ASYNC_F( "unkown exception", res );
        }

        exit( res );
      }
      catch ( std::tr2::fork_in_parent& child2 ) {

        std::tr2::this_thread::block_signal( SIGINT );
        std::tr2::this_thread::block_signal( SIGQUIT );
        std::tr2::this_thread::block_signal( SIGILL );
        std::tr2::this_thread::block_signal( SIGABRT );
        std::tr2::this_thread::block_signal( SIGFPE );
        std::tr2::this_thread::block_signal( SIGSEGV );
        std::tr2::this_thread::block_signal( SIGTERM );
        std::tr2::this_thread::block_signal( SIGPIPE );

        b.wait(); // wait first memeber
        try {
          VT_with_leader_recovery a2( "/tmp/a2" );


          connect_processor<stem::NetTransport> srv( 2010 );

          EXAM_CHECK( addr != stem::badaddr );
        
          list<net_iface> ifaces;

          get_ifaces( back_inserter(ifaces) );

          if ( !ifaces.empty() ) {
            for ( list<net_iface>::iterator f = ifaces.begin(); f != ifaces.end(); ++f ) {
              if ( f->addr.any.sa_family == PF_INET ) {
                f->addr.inet.sin_port = stem::to_net( static_cast<short>(2010) );
                a2.vs_tcp_point( f->addr.inet );
              }
            }
          }

          a2.vs_join( addr, "localhost", 2009 );

          EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 2) );
          std::tr2::this_thread::sleep( std::tr2::milliseconds(200) );

          b3.wait();

          EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 3) );
          std::tr2::this_thread::sleep( std::tr2::milliseconds(200) );

          b4.wait(); // group size 3

          stem::Event ev( EV_EXT_EV_SAMPLE );
          ev.dest( a2.self_id() );

          for ( int j = n_msg1; j < 2 * n_msg1; ++j ) {
            stringstream v;
            v << j;
            ev.value() = v.str();
            a2.Send( ev );
          }

          EXAM_CHECK( a2.wait_msg( std::tr2::milliseconds( max( 500, 20 * n_msg1) ), 3 * n_msg1 ) );
          EXAM_CHECK( a2.wait_flush( std::tr2::milliseconds( max( 500, 20 * n_msg1) ), 1 ) );

          b4.wait(); // group size 3, first 10 events with a1 group leader
          std::tr2::this_thread::sleep( std::tr2::milliseconds(120) );

          // a1 go away, leader failure should be detected and new leader
          // should be elected
          for ( int j = 3 * n_msg1 + n_msg2; j < 3 * n_msg1 + 2 * n_msg2; ++j ) {
            stringstream v;
            v << j;
            ev.value() = v.str();
            a2.Send( ev );
          }

          EXAM_CHECK( a2.wait_msg( std::tr2::milliseconds( max( 500, 20 * n_msg2 ) ), 3 * n_msg1 + 2 * n_msg2) );


          b2.wait();

          b5.wait();

          EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds( max(500, 20 * n_msg2) ), 3) );
          std::tr2::this_thread::sleep( std::tr2::milliseconds(200) );

          b6.wait();


          for ( int j = 3 * n_msg1 + 2 * n_msg2 + 2 * n_msg3; j < 3 * n_msg1 + 2 * n_msg2 + 3 * n_msg3; ++j ) {
            stringstream v;
            v << j;
            ev.value() = v.str();
            a2.Send( ev );
          }

          EXAM_CHECK( a2.wait_msg( std::tr2::milliseconds( max( 500, 20 * n_msg2 ) ), 3 * n_msg1 + 2 * n_msg2 + 3 * n_msg3) );

          b7.wait();
        }
        catch ( ... ) {
          EXAM_ERROR( "unkown exception" );
        }

        int stat = -1;
        EXAM_CHECK( waitpid( child2.pid(), &stat, 0 ) == child2.pid() );
        if ( WIFEXITED(stat) ) {
          EXAM_CHECK( WEXITSTATUS(stat) == 0 );
        } else {
          EXAM_ERROR( "child interrupted" );
        }
      }

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }
    }
    shm.deallocate( &b );
    shm.deallocate( &b2 );
    shm.deallocate( &b3 );
    shm.deallocate( &b4 );
    shm_a.deallocate( &addr );
    seg.deallocate();
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  EXAM_CHECK( system( "[ -s /tmp/a2 ]" ) == 0 );
  EXAM_CHECK( system( "[ -s /tmp/a3 ]" ) == 0 );
  EXAM_CHECK( system( "diff -q /tmp/a2 /tmp/a3" ) == 0 );

  unlink( "/tmp/a1" );
  unlink( "/tmp/a2" );
  unlink( "/tmp/a3" );

  return EXAM_RESULT;
}

} // namespace janus
