// -*- C++ -*- Time-stamp: <10/07/12 11:38:19 ptr>

/*
 *
 * Copyright (c) 2009-2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "vt_operations.h"

#include <iostream>
#include <janus/casual.h>

#include <mt/mutex>
#include <mt/condition_variable>
#include <mt/date_time>
#include <sys/wait.h>
#include <mt/shm.h>
#include <mt/thread>
#include <sockios/sockmgr.h>
#include <stem/NetTransport.h>
#if 0
#include <stem/EvManager.h>
#endif

#include <algorithm>
#include <set>
#include <list>

#include <fstream>
#include <mt/uid.h>
#include <unistd.h>

namespace janus {

using namespace std;

#define EV_FREE      0x9000
#define EV_FREE_SYNC 0x9001

class VTM_one_group_advanced_handler :
    public basic_vs
{
  public:
    VTM_one_group_advanced_handler();
    VTM_one_group_advanced_handler( const stem::addr_type& id );
    ~VTM_one_group_advanced_handler();

    template <class Duration>
    bool wait_group_size( const Duration& rel_time, int _gsize )
      {
        std::tr2::unique_lock<std::tr2::mutex> lk( mtx );
        
        gsize = _gsize;
        
        return cnd.timed_wait( lk, rel_time, gs_status );
      }

    template <class Duration>
    bool wait_msg( const Duration& rel_time, int _n_msg )
      {
        std::tr2::unique_lock<std::tr2::mutex> lk( mtx );
        
        n_msg = _n_msg;

        return cnd.timed_wait( lk, rel_time, msg_status );
      }

    vtime& vt()
      { return basic_vs::vt; }

    virtual xmt::uuid_type vs_pub_recover();
    virtual void vs_resend_from( const xmt::uuid_type&, const stem::addr_type& );
    virtual void vs_pub_view_update();
    virtual void vs_pub_rec( const stem::Event& );
    virtual void vs_pub_flush();

    std::string mess;

    void reset()
      { std::tr2::lock_guard<std::tr2::mutex> lk( mtx ); msg = 0; }

  private:
    void message( const stem::Event& );
    void sync_message( const stem::Event& );

    std::tr2::mutex mtx;
    std::tr2::condition_variable cnd;
    int n_msg;
    int msg;
    int gsize;

    struct _gs_status
    {
        _gs_status( VTM_one_group_advanced_handler& m ) :
            me( m )
          { }

        bool operator()() const;

        VTM_one_group_advanced_handler& me;
    } gs_status;    

    struct _msg_status
    {
        _msg_status( VTM_one_group_advanced_handler& m ) :
            me( m )
          { }

        bool operator()() const;

        VTM_one_group_advanced_handler& me;
    } msg_status;

    std::fstream history;

    DECLARE_RESPONSE_TABLE( VTM_one_group_advanced_handler, janus::basic_vs );
};

VTM_one_group_advanced_handler::VTM_one_group_advanced_handler() :
    basic_vs(),
    msg_status( *this ),
    gs_status( *this ),
    msg(0)
{
  string nm( "/tmp/janus." );
  nm += std::string( self_id() );

  history.open( nm.c_str(), ios_base::in | ios_base::out | ios_base::app );

  enable();
}

VTM_one_group_advanced_handler::VTM_one_group_advanced_handler( const stem::addr_type& id ) :
    basic_vs( /* id */ ),
    msg_status( *this ),
    gs_status( *this ),
    msg(0)
{
  string nm( "/tmp/janus." );
  nm += std::string( /* self_id() */ id );

  history.open( nm.c_str(), ios_base::in | ios_base::out | ios_base::app );

  enable();
}

VTM_one_group_advanced_handler::~VTM_one_group_advanced_handler()
{
  disable();
}

bool VTM_one_group_advanced_handler::_msg_status::operator()() const
{
  return me.msg == me.n_msg;
}

bool VTM_one_group_advanced_handler::_gs_status::operator()() const
{
  return me.vs_group_size() == me.gsize;
}

xmt::uuid_type VTM_one_group_advanced_handler::vs_pub_recover()
{
  stem::Event ev;
  stem::code_type c;
  uint32_t f;

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
    while ( !history.fail() ) {
      stem::__pack_base::__unpack( history, c );
      ev.code( c );
      stem::__pack_base::__unpack( history, f );
      ev.resetf( f );
      stem::__pack_base::__unpack( history, ev.value() );

      if ( !history.fail() ) {
        // basic_vs::sync_call( ev );
        this->Dispatch( ev );
      }
    }
    history.clear();
    history.seekp( 0, ios_base::end );
  }
  catch ( const std::runtime_error& err ) {
    history.clear();
    history.seekp( 0, ios_base::end );
  }

  return xmt::nil_uuid;
}

void VTM_one_group_advanced_handler::vs_resend_from( const xmt::uuid_type&, const stem::addr_type& )
{
}

void VTM_one_group_advanced_handler::vs_pub_view_update()
{
  std::tr2::lock_guard<std::tr2::mutex> lk( mtx );
  cnd.notify_one();
}

void VTM_one_group_advanced_handler::vs_pub_rec( const stem::Event& ev )
{
  stem::__pack_base::__pack( history, ev.code() );
  stem::__pack_base::__pack( history, ev.flags() );
  stem::__pack_base::__pack( history, ev.value() );
}

void VTM_one_group_advanced_handler::vs_pub_flush()
{
  std::tr2::lock_guard<std::tr2::mutex> lk( mtx );
  cnd.notify_one();
}

void VTM_one_group_advanced_handler::message( const stem::Event& ev )
{
  stem::Event sync( ev );
  sync.code( EV_FREE_SYNC );

  vs( sync );
}

void VTM_one_group_advanced_handler::sync_message( const stem::Event& ev )
{
  mess = ev.value();

  std::tr2::lock_guard<std::tr2::mutex> lk( mtx );
  ++msg;
  cnd.notify_one();
}

DEFINE_RESPONSE_TABLE( VTM_one_group_advanced_handler )
  EV_EDS( ST_NULL, EV_FREE, message )
  EV_EDS( ST_NULL, EV_FREE_SYNC, sync_message )
END_RESPONSE_TABLE

int EXAM_IMPL(vtime_operations::VT_one_group_replay)
{
  for (int i = 0;i < 1000;++i) {
  stem::addr_type a1_stored;
  stem::addr_type a2_stored;
  stem::addr_type a3_stored;

  VTM_one_group_advanced_handler a1;
  VTM_one_group_advanced_handler a2;

  a1_stored = a1.self_id();
  a2_stored = a2.self_id();

  a1.vs_join( stem::badaddr );
  a2.vs_join( a1.self_id() );

  EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 2 ) );
  EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 2 ) );

  {
    VTM_one_group_advanced_handler a3;

    a3_stored = a3.self_id();
    a3.vs_join( a2.self_id() );

    EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 3 ) );
    EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 3 ) );
    
    stem::Event ev( EV_FREE );
    ev.value() = "message";
    ev.dest( a1.self_id() );

    a1.Send( ev ); // simulate outer event

    EXAM_CHECK( a1.wait_msg( std::tr2::milliseconds(500), 1 ) );
    EXAM_CHECK( a1.mess == "message" );
    EXAM_CHECK( a2.wait_msg( std::tr2::milliseconds(500), 1 ) );
    EXAM_CHECK( a2.mess == "message" );
    EXAM_CHECK( a3.wait_msg( std::tr2::milliseconds(500), 1 ) );
    EXAM_CHECK( a3.mess == "message" );
  }

  a1.vs_send_flush();

  {
    EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 2 ) );
    EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 2 ) );

    VTM_one_group_advanced_handler a3( a3_stored );

    a3.vs_join( a2.self_id() );

    EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 3 ) );
    EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 3 ) );
    EXAM_CHECK( a3.wait_group_size( std::tr2::milliseconds(500), 3 ) );

    EXAM_CHECK( a3.wait_msg( std::tr2::milliseconds(500), 1 ) );
    EXAM_CHECK( a3.mess == "message" );
  }

  unlink( (std::string( "/tmp/janus." ) + std::string(a1_stored) ).c_str() );
  unlink( (std::string( "/tmp/janus." ) + std::string(a2_stored) ).c_str() );
  unlink( (std::string( "/tmp/janus." ) + std::string(a3_stored) ).c_str() );
  if ( EXAM_RESULT ) {
    break;
  }
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::VT_one_group_late_replay)
{
  for (int i = 0;i < 1000;++i) {
  stem::addr_type a1_stored;
  stem::addr_type a2_stored;
  stem::addr_type a3_stored;

  VTM_one_group_advanced_handler a1;
  VTM_one_group_advanced_handler a2;

  a1_stored = a1.self_id();
  a2_stored = a2.self_id();

  a1.vs_join( stem::badaddr );
  a2.vs_join( a1.self_id() );

  EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 2 ) );
  EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 2 ) );

  {
    VTM_one_group_advanced_handler a3;

    a3_stored = a3.self_id();
    a3.vs_join( a2.self_id() );

    EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 3 ) );
    EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 3 ) );
    
    stem::Event ev( EV_FREE );
    ev.value() = "message";
    ev.dest( a1.self_id() );

    a1.Send( ev ); // simulate outer event

    EXAM_CHECK( a1.wait_msg( std::tr2::milliseconds(500), 1 ) );
    EXAM_CHECK( a1.mess == "message" );
    EXAM_CHECK( a2.wait_msg( std::tr2::milliseconds(500), 1 ) );
    EXAM_CHECK( a2.mess == "message" );
    EXAM_CHECK( a3.wait_msg( std::tr2::milliseconds(500), 1 ) );
    EXAM_CHECK( a3.mess == "message" );
  }

  a1.vs_send_flush();

  {
    EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 2 ) );
    EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 2 ) );

    stem::Event ev( EV_FREE );
    ev.value() = "extra message";
    ev.dest( a1.self_id() );

    a1.Send( ev );

    EXAM_CHECK( a1.wait_msg( std::tr2::milliseconds(500), 2 ) );
    EXAM_CHECK( a1.mess == "extra message" );
    EXAM_CHECK( a2.wait_msg( std::tr2::milliseconds(500), 2 ) );
    EXAM_CHECK( a2.mess == "extra message" );
  }

  {
    VTM_one_group_advanced_handler a3( a3_stored );

    a3.vs_join( a2.self_id() );

    EXAM_CHECK( a1.wait_group_size( std::tr2::milliseconds(500), 3 ) );
    EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 3 ) );
    EXAM_CHECK( a3.wait_group_size( std::tr2::milliseconds(500), 3 ) );

    EXAM_CHECK( a3.wait_msg( std::tr2::milliseconds(500), 1 ) );
    EXAM_CHECK( a3.mess == "message" );
  }

  unlink( (std::string( "/tmp/janus." ) + std::string(a1_stored) ).c_str() );
  unlink( (std::string( "/tmp/janus." ) + std::string(a2_stored) ).c_str() );
  unlink( (std::string( "/tmp/janus." ) + std::string(a3_stored) ).c_str() );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::VT_one_group_network)
{
  try {
    xmt::shm_alloc<0> seg;
    seg.allocate( 70000, 4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    xmt::allocator_shm<std::tr2::barrier_ip,0> shm;
    std::tr2::barrier_ip& b = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip();
    std::tr2::barrier_ip& b2 = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip();

    stem::addr_type a_stored;

    try {
      tr2::this_thread::fork();
      int res = 0;

      {
        VTM_one_group_advanced_handler a1;

        a_stored = a1.self_id();

        a1.vs_join( stem::badaddr );

        connect_processor<stem::NetTransport> srv( 2009 );

        EXAM_CHECK_ASYNC_F( srv.is_open(), res );

        a1.set_default();

        b.wait();
        b2.wait();
      }

      unlink( (std::string( "/tmp/janus." ) + std::string(a_stored) ).c_str() );

      exit( res );
    }
    catch ( std::tr2::fork_in_parent& child ) {
      b.wait();
      {
        VTM_one_group_advanced_handler a2;

        a_stored = a2.self_id();

        stem::NetTransportMgr mgr;

        stem::addr_type a1 = mgr.open( "localhost", 2009 );

        EXAM_CHECK( a1 != stem::badaddr );
        
        a2.vs_join( a1 );

        EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 2 ) );
      }
      b2.wait();

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }

      unlink( (std::string( "/tmp/janus." ) + std::string(a_stored) ).c_str() );
    }
    shm.deallocate( &b );
    shm.deallocate( &b2 );
    seg.deallocate();
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::VT_one_group_access_point)
{
  for (int p = 0;p < 100;++p) {
    try {
      xmt::shm_alloc<0> seg;
      seg.allocate( 70000, 4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

      xmt::allocator_shm<std::tr2::barrier_ip,0> shm;
      xmt::allocator_shm<stem::addr_type,0> shm_a;

      std::tr2::barrier_ip& b = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip();
      std::tr2::barrier_ip& b2 = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip();
      std::tr2::barrier_ip& b3 = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip();
      stem::addr_type& addr1 = *new ( shm_a.allocate( 1 ) ) stem::addr_type();

      try {
        tr2::this_thread::fork();
        int res = 0;

        stem::addr_type me;

        {
          VTM_one_group_advanced_handler a1;

          a1.vs_join( stem::badaddr );

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

          a1.set_default();

          addr1 = a1.self_id();
          me = a1.self_id();

          b.wait();
          b2.wait();

          EXAM_CHECK_ASYNC_F( a1.wait_group_size( std::tr2::milliseconds(500), 2 ), res );

          EXAM_CHECK_ASYNC_F( a1.wait_msg( std::tr2::milliseconds(500), 1 ), res );
          EXAM_CHECK_ASYNC_F( a1.mess == "message", res );

          b3.wait();
        }

        unlink( (std::string( "/tmp/janus." ) + std::string(me) ).c_str() );

        exit( res );
      }
      catch ( std::tr2::fork_in_parent& child ) {
        b.wait();

        stem::addr_type me;
        {
          VTM_one_group_advanced_handler a2;
          me = a2.self_id();
          connect_processor<stem::NetTransport> srv( 2010 );

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

          a2.vs_join( addr1, "localhost", 2009 );
          b2.wait();

          EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 2 ) );

          stem::Event ev( EV_FREE );
          ev.value() = "message";
          ev.dest( a2.self_id() );

          a2.Send( ev ); // simulate outer event

          EXAM_CHECK( a2.wait_msg( std::tr2::milliseconds(500), 1 ) );
          EXAM_CHECK( a2.mess == "message" );

          b3.wait();
        }

        int stat = -1;
        EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
        if ( WIFEXITED(stat) ) {
          EXAM_CHECK( WEXITSTATUS(stat) == 0 );
        } else {
          EXAM_ERROR( "child interrupted" );
        }

        unlink( (std::string( "/tmp/janus." ) + std::string(me) ).c_str() );
      }
      shm.deallocate( &b );
      shm.deallocate( &b2 );
      shm.deallocate( &b3 );
      shm_a.deallocate( &addr1 );
      seg.deallocate();
    }
    catch ( xmt::shm_bad_alloc& err ) {
      EXAM_ERROR( err.what() );
    }
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::VT_one_group_access_point_ex)
{
  try {
    xmt::shm_alloc<0> seg;
    seg.allocate( 70000, 4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    xmt::allocator_shm<std::tr2::barrier_ip,0> shm;
    xmt::allocator_shm<stem::addr_type,0> shm_a;

    std::tr2::barrier_ip& b = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip();
    std::tr2::barrier_ip& b2 = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip( 3 );
    std::tr2::barrier_ip& b3 = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip();
    std::tr2::barrier_ip& b4 = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip( 3 );
    std::tr2::barrier_ip& b5 = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip( 4 );
    std::tr2::barrier_ip& b6 = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip( 4 );
    std::tr2::barrier_ip& b7 = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip( 4 );
    std::tr2::barrier_ip& b8 = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip( 4 );
    std::tr2::barrier_ip& b9 = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip( 4 );
    stem::addr_type& addr1 = *new ( shm_a.allocate( 1 ) ) stem::addr_type();
    stem::addr_type& addr2 = *new ( shm_a.allocate( 1 ) ) stem::addr_type();
    stem::addr_type& addr3 = *new ( shm_a.allocate( 1 ) ) stem::addr_type();
    stem::addr_type& addr4 = *new ( shm_a.allocate( 1 ) ) stem::addr_type();


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
        VTM_one_group_advanced_handler a1;

        addr1 = a1.self_id();

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

        b4.wait();
        EXAM_CHECK_ASYNC_F( a1.wait_group_size( std::tr2::milliseconds(500), 3 ), res );
        b5.wait();

        b6.wait();
        EXAM_CHECK_ASYNC_F( a1.wait_group_size( std::tr2::milliseconds(500), 4 ), res );
        b7.wait();

        b8.wait();
        EXAM_CHECK_ASYNC_F( a1.wait_msg( std::tr2::milliseconds(500), 1 ), res );
        EXAM_CHECK_ASYNC_F( a1.mess == "message", res );
        b9.wait();
      }
      catch ( ... ) {
        EXAM_ERROR_ASYNC_F( "unkown exception", res );
      }

      unlink( (std::string( "/tmp/janus." ) + std::string(addr1) ).c_str() );

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
          VTM_one_group_advanced_handler a3;

          addr3 = a3.self_id();

          connect_processor<stem::NetTransport> srv( 2011 );

          EXAM_CHECK_ASYNC_F( srv.is_open(), res );

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

          a3.vs_join( addr1, "localhost", 2009 );

          b4.wait();
          EXAM_CHECK_ASYNC_F( a3.wait_group_size( std::tr2::milliseconds(500), 3 ), res );
          b5.wait();

          b6.wait();
          EXAM_CHECK_ASYNC_F( a3.wait_group_size( std::tr2::milliseconds(500), 4 ), res );
          b7.wait();

          b8.wait();
          EXAM_CHECK_ASYNC_F( a3.wait_msg( std::tr2::milliseconds(500), 1 ), res );
          EXAM_CHECK_ASYNC_F( a3.mess == "message", res );
          b9.wait();

        }
        catch ( ... ) {
          EXAM_ERROR_ASYNC_F( "unkown exception", res );
        }

        unlink( (std::string( "/tmp/janus." ) + std::string(addr3) ).c_str() );

        exit( res );
      }
      catch ( std::tr2::fork_in_parent& child3 ) {
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

          b5.wait();
          try { 
            VTM_one_group_advanced_handler a4;

            addr4 = a4.self_id();

            connect_processor<stem::NetTransport> srv( 2012 );

            EXAM_CHECK_ASYNC_F( srv.is_open(), res );

            list<net_iface> ifaces;

            get_ifaces( back_inserter(ifaces) );

            if ( !ifaces.empty() ) {
              for ( list<net_iface>::iterator f = ifaces.begin(); f != ifaces.end(); ++f ) {
                if ( f->addr.any.sa_family == PF_INET ) {
                  f->addr.inet.sin_port = stem::to_net( static_cast<short>(2012) );
                  a4.vs_tcp_point( f->addr.inet );
                }
              }
            }

            a4.vs_join( addr2, "localhost", 2010 );

            b6.wait();
            EXAM_CHECK_ASYNC_F( a4.wait_group_size( std::tr2::milliseconds(500), 4 ), res );
            b7.wait();

            stem::Event ev( EV_FREE );
            ev.value() = "message";
            ev.dest( a4.self_id() );

            a4.Send( ev ); // simulate outer event

            b8.wait();
            EXAM_CHECK_ASYNC_F( a4.wait_msg( std::tr2::milliseconds(500), 1 ), res );
            EXAM_CHECK_ASYNC_F( a4.mess == "message", res );
            b9.wait();

          }
          catch ( ... ) {
            EXAM_ERROR_ASYNC_F( "unkown exception", res );
          }

          unlink( (std::string( "/tmp/janus." ) + std::string(addr4) ).c_str() );

          exit( res );
        } catch ( std::tr2::fork_in_parent& child2 ) {

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
            VTM_one_group_advanced_handler a2;

            addr2 = a2.self_id();

            connect_processor<stem::NetTransport> srv( 2010 );

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

            a2.vs_join( addr1, "localhost", 2009 );

            // a2 join to group:
            EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 2 ) );

            b3.wait();
            // a3 join to group:
            b4.wait();
            EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 3 ) );
            b5.wait();

            b6.wait();
            EXAM_CHECK( a2.wait_group_size( std::tr2::milliseconds(500), 4 ) );
            b7.wait();

            b8.wait();
            EXAM_CHECK( a2.wait_msg( std::tr2::milliseconds(500), 1 ) );
            EXAM_CHECK( a2.mess == "message" );
            b9.wait();
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
        EXAM_CHECK( waitpid( child3.pid(), &stat, 0 ) == child3.pid() );
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

      unlink( (std::string( "/tmp/janus." ) + std::string(addr2) ).c_str() );
    }

    shm.deallocate( &b );
    shm.deallocate( &b2 );
    shm.deallocate( &b3 );
    shm.deallocate( &b4 );
    shm.deallocate( &b5 );
    shm.deallocate( &b6 );
    shm.deallocate( &b7 );
    shm.deallocate( &b8 );
    shm.deallocate( &b9 );
    shm_a.deallocate( &addr1 );
    shm_a.deallocate( &addr2 );
    shm_a.deallocate( &addr3 );
    shm_a.deallocate( &addr4 );
    seg.deallocate();
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

} // namespace janus
