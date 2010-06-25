// -*- C++ -*- Time-stamp: <10/06/24 22:12:27 ptr>

/*
 *
 * Copyright (c) 2010
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "vt_operations.h"
#include "leader.h"

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

VT_with_leader::VT_with_leader( const char* nm ) :
    torder_vs(),
    f( nm ),
    name( nm ),
    flushed( false )
{
  enable();
}

VT_with_leader::~VT_with_leader()
{
  disable();
}

xmt::uuid_type VT_with_leader::vs_pub_recover()
{
  return xmt::nil_uuid;
}

void VT_with_leader::vs_resend_from( const xmt::uuid_type&, const stem::addr_type& )
{
}

void VT_with_leader::vs_pub_view_update()
{
  torder_vs::vs_pub_view_update();
}

void VT_with_leader::vs_pub_rec( const stem::Event& )
{
  // cerr << name << endl;
}

void VT_with_leader::vs_pub_flush()
{
  flushed = true;
}

void VT_with_leader::vs_pub_tord_rec( const stem::Event& ev )
{
  f << ev.value() << '\n';
}

#define EV_EXT_EV_SAMPLE      0x9010
#define EV_VS_EV_SAMPLE       0x9011
#define EV_VS_EV_SAMPLE2      0x9012

void VT_with_leader::message( const stem::Event& ev )
{
  // retranslate this message within virtual synchrony group
  // with total order of events

  stem::Event sync( EV_VS_EV_SAMPLE );

  sync.value() = ev.value();

  vs_torder( sync );
}

void VT_with_leader::sync_message( const stem::Event& ev )
{
  f << ev.value() << '\n';
}

DEFINE_RESPONSE_TABLE( VT_with_leader )
  EV_EDS( ST_NULL, EV_EXT_EV_SAMPLE, message )
  EV_EDS( ST_NULL, EV_VS_EV_SAMPLE, sync_message )
END_RESPONSE_TABLE

int EXAM_IMPL(vtime_operations::leader)
{
  try {
    xmt::shm_alloc<0> seg;
    seg.allocate( 70000, 4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    xmt::allocator_shm<std::tr2::barrier_ip,0> shm;
    xmt::allocator_shm<stem::addr_type,0> shm_a;

    std::tr2::barrier_ip& b = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip();
    std::tr2::barrier_ip& b2 = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip( 3 );
    std::tr2::barrier_ip& b3 = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip();
    stem::addr_type& addr = *new ( shm_a.allocate( 1 ) ) stem::addr_type();

    addr = stem::badaddr;

    stem::addr_type a_stored;

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
        VT_with_leader a1( "/tmp/a1" );

        a_stored = a1.self_id();

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
            }// else if ( f->addr.any.sa_family == PF_INET6 ) {
            // refzone.inet6_point( f->addr.inet.sin_addr.s_addr );
            // }
          }
        }

        // a1.set_default();
        a1.vs_join( stem::badaddr );

        b.wait();

        int i;
        for ( i = 0; i < 5; ++i ) {
          if ( a1.vs_group_size() != 3 ) {
            std::tr2::this_thread::sleep( std::tr2::milliseconds(100) );
          } else {
            break;
          }
        }

        EXAM_CHECK_ASYNC_F( i < 5, res );

        stem::Event ev( EV_EXT_EV_SAMPLE );
        ev.dest( a1.self_id() );

        for ( int j = 0; j < 100; ++j ) {
          stringstream v;
          v << j;
          ev.value() = v.str();
          a1.Send( ev );
          // std::tr2::this_thread::sleep( std::tr2::milliseconds(20) );
        }

        std::tr2::this_thread::sleep( std::tr2::milliseconds(1200) );
        b2.wait();
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
          VT_with_leader a3( "/tmp/a3" );

          a_stored = a3.self_id();

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
              }// else if ( f->addr.any.sa_family == PF_INET6 ) {
              // refzone.inet6_point( f->addr.inet.sin_addr.s_addr );
              // }
            }
          }

          a3.vs_join( addr, "localhost", 2009 );

          int i;
          for ( i = 0; i < 5; ++i ) {
            if ( a3.vs_group_size() != 3 ) {
              std::tr2::this_thread::sleep( std::tr2::milliseconds(100) );
            } else {
              break;
            }
          }

          EXAM_CHECK_ASYNC_F( i < 5, res );

          stem::Event ev( EV_EXT_EV_SAMPLE );
          ev.dest( a3.self_id() );

          for ( int j = 300; j < 400; ++j ) {
            stringstream v;
            v << j;
            ev.value() = v.str();
            a3.Send( ev );
            // std::tr2::this_thread::sleep( std::tr2::milliseconds(20) );
          }

          std::tr2::this_thread::sleep( std::tr2::milliseconds(1200) );
          b2.wait();
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
          VT_with_leader a2( "/tmp/a2" );

          a_stored = a2.self_id();

          connect_processor<stem::NetTransport> srv( 2010 );

          EXAM_CHECK( addr != stem::badaddr );
        
          list<net_iface> ifaces;

          get_ifaces( back_inserter(ifaces) );

          if ( !ifaces.empty() ) {
            for ( list<net_iface>::iterator f = ifaces.begin(); f != ifaces.end(); ++f ) {
              if ( f->addr.any.sa_family == PF_INET ) {
                f->addr.inet.sin_port = stem::to_net( static_cast<short>(2010) );
                a2.vs_tcp_point( f->addr.inet );
              }// else if ( f->addr.any.sa_family == PF_INET6 ) {
              // refzone.inet6_point( f->addr.inet.sin_addr.s_addr );
              // }
            }
          }

          a2.vs_join( addr, "localhost", 2009 );

          int i;
          for ( i = 0; i < 5; ++i ) {
            if ( a2.vs_group_size() != 2 ) {
              std::tr2::this_thread::sleep( std::tr2::milliseconds(100) );
            } else {
              break;
            }
          }

          EXAM_CHECK( i < 5 );

          b3.wait();

          for ( i = 0; i < 5; ++i ) {
            if ( a2.vs_group_size() != 3 ) {
              std::tr2::this_thread::sleep( std::tr2::milliseconds(100) );
            } else {
              break;
            }
          }

          EXAM_CHECK( i < 5 );

          stem::Event ev( EV_EXT_EV_SAMPLE );
          ev.dest( a2.self_id() );

          for ( int j = 100; j < 200; ++j ) {
            stringstream v;
            v << j;
            ev.value() = v.str();
            a2.Send( ev );
            // std::tr2::this_thread::sleep( std::tr2::milliseconds(20) );
          }

          std::tr2::this_thread::sleep( std::tr2::milliseconds(1200) );
          b2.wait();
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
    shm_a.deallocate( &addr );
    seg.deallocate();
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  EXAM_CHECK( system( "diff -q /tmp/a1 /tmp/a2 && diff -q /tmp/a2 /tmp/a3" ) == 0 );

  unlink( "/tmp/a1" );
  unlink( "/tmp/a2" );
  unlink( "/tmp/a3" );

  return EXAM_RESULT;
}

int EXAM_IMPL(vtime_operations::leader_fail)
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
    stem::addr_type& addr = *new ( shm_a.allocate( 1 ) ) stem::addr_type();

    addr = stem::badaddr;

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
        VT_with_leader a1( "/tmp/a1" );

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
            }// else if ( f->addr.any.sa_family == PF_INET6 ) {
            // refzone.inet6_point( f->addr.inet.sin_addr.s_addr );
            // }
          }
        }

        // a1.set_default();
        a1.vs_join( stem::badaddr );

        b.wait();

        int i;
        for ( i = 0; i < 5; ++i ) {
          if ( a1.vs_group_size() != 3 ) {
            std::tr2::this_thread::sleep( std::tr2::milliseconds(100) );
          } else {
            break;
          }
        }

        EXAM_CHECK_ASYNC_F( i < 5, res );

        b4.wait(); // group size 3

        // a1 is group leader here
        stem::Event ev( EV_EXT_EV_SAMPLE );
        ev.dest( a1.self_id() );

        for ( int j = 0; j < 10; ++j ) {
          stringstream v;
          v << j;
          ev.value() = v.str();
          a1.Send( ev );
        }

        b4.wait(); // group size 3, align with others
        std::tr2::this_thread::sleep( std::tr2::milliseconds(520) );
      }
      catch ( ... ) {
        EXAM_ERROR_ASYNC_F( "unkown exception", res );
      }

      // std::tr2::this_thread::sleep( std::tr2::milliseconds(120) );
      b2.wait(); // should be here: after dtor of a1

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
          VT_with_leader a3( "/tmp/a3" );

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
              }// else if ( f->addr.any.sa_family == PF_INET6 ) {
              // refzone.inet6_point( f->addr.inet.sin_addr.s_addr );
              // }
            }
          }

          a3.vs_join( addr, "localhost", 2009 );

          int i;
          for ( i = 0; i < 5; ++i ) {
            if ( a3.vs_group_size() != 3 ) {
              std::tr2::this_thread::sleep( std::tr2::milliseconds(100) );
            } else {
              break;
            }
          }

          EXAM_CHECK_ASYNC_F( i < 5, res );

          b4.wait(); // group size 3

          stem::Event ev( EV_EXT_EV_SAMPLE );
          ev.dest( a3.self_id() );

          for ( int j = 300; j < 310; ++j ) {
            stringstream v;
            v << j;
            ev.value() = v.str();
            a3.Send( ev );
            // std::tr2::this_thread::sleep( std::tr2::milliseconds(20) );
          }

          b4.wait(); // group size 3, first 10 events with a1 group leader

          // a1 go away, leader failure should be detected and new leader
          // should be elected
          for ( int j = 310; j < 400; ++j ) {
            stringstream v;
            v << j;
            ev.value() = v.str();
            a3.Send( ev );
            // std::tr2::this_thread::sleep( std::tr2::milliseconds(20) );
          }

          std::tr2::this_thread::sleep( std::tr2::milliseconds(520) );
          b2.wait();
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
          VT_with_leader a2( "/tmp/a2" );

          connect_processor<stem::NetTransport> srv( 2010 );

          EXAM_CHECK( addr != stem::badaddr );
        
          list<net_iface> ifaces;

          get_ifaces( back_inserter(ifaces) );

          if ( !ifaces.empty() ) {
            for ( list<net_iface>::iterator f = ifaces.begin(); f != ifaces.end(); ++f ) {
              if ( f->addr.any.sa_family == PF_INET ) {
                f->addr.inet.sin_port = stem::to_net( static_cast<short>(2010) );
                a2.vs_tcp_point( f->addr.inet );
              }// else if ( f->addr.any.sa_family == PF_INET6 ) {
              // refzone.inet6_point( f->addr.inet.sin_addr.s_addr );
              // }
            }
          }

          a2.vs_join( addr, "localhost", 2009 );

          int i;
          for ( i = 0; i < 5; ++i ) {
            if ( a2.vs_group_size() != 2 ) {
              std::tr2::this_thread::sleep( std::tr2::milliseconds(100) );
            } else {
              break;
            }
          }

          EXAM_CHECK( i < 5 );

          b3.wait();

          for ( i = 0; i < 5; ++i ) {
            if ( a2.vs_group_size() != 3 ) {
              std::tr2::this_thread::sleep( std::tr2::milliseconds(100) );
            } else {
              break;
            }
          }

          EXAM_CHECK( i < 5 );

          b4.wait(); // group size 3

          stem::Event ev( EV_EXT_EV_SAMPLE );
          ev.dest( a2.self_id() );

          for ( int j = 100; j < 110; ++j ) {
            stringstream v;
            v << j;
            ev.value() = v.str();
            a2.Send( ev );
            // std::tr2::this_thread::sleep( std::tr2::milliseconds(20) );
          }

          b4.wait(); // group size 3, first 10 events with a1 group leader

          // a1 go away, leader failure should be detected and new leader
          // should be elected
          for ( int j = 110; j < 200; ++j ) {
            stringstream v;
            v << j;
            ev.value() = v.str();
            a2.Send( ev );
            // std::tr2::this_thread::sleep( std::tr2::milliseconds(20) );
          }

          std::tr2::this_thread::sleep( std::tr2::milliseconds(520) );
          b2.wait();
        }
        catch ( ... ) {
          EXAM_ERROR( "unknown exception" );
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

TO_object::TO_object() :
    joined( false ),
    joined2( false ),
    status( *this ),
    status2( *this )
{
  enable();
}

TO_object::~TO_object()
{
  disable();
}

bool TO_object::_status::operator()() const
{
  return me.joined;
}

bool TO_object::_status2::operator()() const
{
  return me.joined2;
}

xmt::uuid_type TO_object::vs_pub_recover()
{
  // cerr << HERE << endl;
  return xmt::nil_uuid;
}

void TO_object::vs_resend_from( const xmt::uuid_type&, const stem::addr_type& )
{
  // cerr << HERE << endl;
}

void TO_object::vs_pub_view_update()
{
  // cerr << HERE << endl;

  torder_vs::vs_pub_view_update();

  // std::tr2::lock_guard<std::tr2::mutex> lk( mtx_joined );
  // joined = true;
  // cnd.notify_one();
}

void TO_object::vs_pub_rec( const stem::Event& )
{
  // cerr << name << endl;
}

void TO_object::vs_pub_flush()
{
}

void TO_object::vs_pub_tord_rec( const stem::Event& ev )
{
}

void TO_object::message( const stem::Event& )
{
  // cerr << HERE << endl;

  std::tr2::lock_guard<std::tr2::mutex> lk( mtx_joined );
  joined = true;
  cnd.notify_one();
}

void TO_object::message2( const stem::Event& )
{
  // cerr << HERE << endl;

  std::tr2::lock_guard<std::tr2::mutex> lk( mtx_joined );
  joined2 = true;
  cnd.notify_one();
}

int EXAM_IMPL(vtime_operations::lock_and_torder)
{
  throw exam::skip_exception(); // implementation changed,
                                // this test can't pass

  TO_object a;
  TO_object b;

  a.vs_join( stem::badaddr );

  // EXAM_CHECK( a.wait( std::tr2::milliseconds(200) ) );

  b.vs_join( a.self_id() );

  for ( int i = 0; (i < 100) && (b.vs_group_size() < 2); ++i ) {
    std::tr2::this_thread::sleep( std::tr2::milliseconds(100) );
  }

  // EXAM_CHECK( b.wait( std::tr2::milliseconds(1000) ) );

  // cerr << HERE << ' ' << a.is_leader() << endl;
  // cerr << HERE << ' ' << b.is_leader() << endl;

  a.PushState( 0x10000 /* basic_vs::VS_ST_LOCKED */ );

  stem::Event ev( EV_VS_EV_SAMPLE );

  EXAM_CHECK( a.vs_torder( ev ) != 0 );

  std::tr2::this_thread::sleep( std::tr2::milliseconds(100) );

  EXAM_CHECK( a.is_leader() );
  EXAM_CHECK( !b.is_leader() );

  a.PopState();

  stem::Event ev2( EV_VS_EV_SAMPLE2 );

  EXAM_CHECK( a.vs_torder( ev2 ) == 0 );

  EXAM_CHECK( a.is_leader() );
  EXAM_CHECK( !b.is_leader() );

  EXAM_CHECK( a.wait( std::tr2::milliseconds(200) ) );
  EXAM_CHECK( b.wait( std::tr2::milliseconds(200) ) );

  EXAM_CHECK( a.wait2( std::tr2::milliseconds(200) ) );
  EXAM_CHECK( b.wait2( std::tr2::milliseconds(200) ) );

  return EXAM_RESULT;
}

DEFINE_RESPONSE_TABLE( TO_object )
  EV_EDS( ST_NULL, EV_VS_EV_SAMPLE, message )
  EV_EDS( ST_NULL, EV_VS_EV_SAMPLE2, message2 )
END_RESPONSE_TABLE


} // namespace janus
