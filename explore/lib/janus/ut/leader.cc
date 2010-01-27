// -*- C++ -*- Time-stamp: <10/01/27 20:30:04 ptr>

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
    f( nm )
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
}

void VT_with_leader::vs_pub_rec( const stem::Event& )
{
}

void VT_with_leader::vs_pub_flush()
{
}

#define EV_EXT_EV_SAMPLE      0x9010
#define EV_VS_EV_SAMPLE       0x9011

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

        std::tr2::this_thread::sleep( std::tr2::milliseconds(120) );
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

          std::tr2::this_thread::sleep( std::tr2::milliseconds(120) );
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

          std::tr2::this_thread::sleep( std::tr2::milliseconds(120) );
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

} // namespace janus
