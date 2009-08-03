// -*- C++ -*- Time-stamp: <09/08/03 15:58:23 ptr>

/*
 *
 * Copyright (c) 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "stem_perf.h"
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

std::tr2::mutex Tester::lock;
std::tr2::condition_variable Tester::cnd;
int Tester::visits = 0;

const int EV_TEST = 0x1000;
const int EV_SETPEER = 0x1001;
const int port = 6480;

static const int N = 100000;

Tester::Tester() :
    peer( badaddr )
{
  EventHandler::enable();
}

Tester::Tester( stem::addr_type id ) :
    EventHandler( id ),
    peer( badaddr )
{
  EventHandler::enable();
}

Tester::Tester( stem::addr_type id, const char* info ) :
    EventHandler( id, info ),
    peer( badaddr )
{
  EventHandler::enable();
}

bool Tester::n_cnt()
{ return visits == N; }

void Tester::handler1( const stem::Event& )
{
  lock_guard<mutex> lk( lock );
  // cerr << visits << endl;
  if ( ++visits == N ) {
    cnd.notify_one();
  }
}

void Tester::handler2( const stem::Event& ev )
{
  lock_guard<mutex> lk( peer_lock );

  peer = ev.src();

  peer_cnd.notify_one();
}

bool Tester::wait_peer()
{
  pred p( *this );

  unique_lock<mutex> lk( peer_lock );
  return peer_cnd.timed_wait( lk, std::tr2::milliseconds( 500 ), p );
}

void Tester::send()
{
  Event ev( EV_TEST );
  ev.dest( peer );

  for ( int i = 0; i < N; ++i ) {
    Send( ev );
  }
}

DEFINE_RESPONSE_TABLE( Tester )
  EV_EDS( ST_NULL, EV_TEST, handler1 )
  EV_EDS( ST_NULL, EV_SETPEER, handler2 )
END_RESPONSE_TABLE

Busy::Busy( stem::addr_type id ) :
    EventHandler( id ),
    cnt( 0 ),
    cnt_lim( 10 )
{
  EventHandler::enable();
}

bool Busy::wait()
{
  pred p( *this );

  unique_lock<mutex> lk( lock );
  return cnd.timed_wait( lk, std::tr2::seconds( 5 ), p );
}

void Busy::handler( const stem::Event& )
{
  this_thread::sleep( milliseconds(100) );

  lock_guard<mutex> lk(lock);
  if ( ++cnt == cnt_lim ) {
    cnd.notify_one();
  }
}

DEFINE_RESPONSE_TABLE( Busy )
  EV_EDS( ST_NULL, EV_TEST, handler )
END_RESPONSE_TABLE

stem_perf::stem_perf()
{
}

stem_perf::~stem_perf()
{
}

int EXAM_IMPL(stem_perf::local)
{
  Tester t1;
  Tester t2;

  Event ev( EV_TEST );
  ev.dest( t2.self_id() );

  for ( int i = 0; i < N; ++i ) {
    t1.Send( ev );
  }

  std::tr2::unique_lock<std::tr2::mutex> lk(Tester::lock);
  EXAM_CHECK( Tester::cnd.timed_wait( lk, std::tr2::milliseconds( 1500 ), Tester::n_cnt ) );

  Tester::visits = 0;

  return EXAM_RESULT;
}

int EXAM_IMPL(stem_perf::local_too)
{
  std::connect_processor<NetTransport> srv( port );
  stem::NetTransportMgr mgr;

  Tester t1;
  Tester t2;

  t2.set_default();

  stem::addr_type zero = mgr.open( "localhost", port );

  Event ev( EV_TEST );
  ev.dest( zero );

  for ( int i = 0; i < N; ++i ) {
    t1.Send( ev );
  }

  std::tr2::unique_lock<std::tr2::mutex> lk(Tester::lock);
  EXAM_CHECK( Tester::cnd.timed_wait( lk, std::tr2::milliseconds( 1500 ), Tester::n_cnt ) );

  mgr.close();
  mgr.join();
  srv.close();
  srv.wait();

  Tester::visits = 0;

  return EXAM_RESULT;
}

int EXAM_IMPL(stem_perf::net_loopback)
{
  try {
    xmt::shm_alloc<0> seg;
    seg.allocate( 70000, 4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    xmt::allocator_shm<std::tr2::barrier_ip,0> shm;
    // xmt::allocator_shm<std::tr2::condition_variable_ip,0> shmcnd;
    // xmt::allocator_shm<std::tr2::mutex_ip,0> shmm;
    // xmt::allocator_shm<int,0> shmi;
    xmt::allocator_shm<std::tr2::condition_event_ip,0> shme;

    std::tr2::barrier_ip& b = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip();
    // std::tr2::condition_variable_ip& c = *new ( shmcnd.allocate( 1 ) ) std::tr2::condition_variable_ip();
    // std::tr2::mutex_ip& m = *new ( shmm.allocate( 1 ) ) std::tr2::mutex_ip();
    // int& j = *new ( shmi.allocate( 1 ) ) int();
    std::tr2::condition_event_ip& c = *new ( shme.allocate( 1 ) ) std::tr2::condition_event_ip();
    // j = 0;

    try {
      std::tr2::this_thread::fork();

      int flag = 0;

      try {
        std::tr2::this_thread::block_signal( SIGINT );

        std::connect_processor<NetTransport> srv( port );

        EXAM_CHECK_ASYNC_F( srv.good(), flag );
        EXAM_CHECK_ASYNC_F( srv.is_open(), flag );

        sigset_t signal_mask;

        sigemptyset( &signal_mask );
        sigaddset( &signal_mask, SIGINT );

        Tester t2;
        t2.set_default();

        b.wait();

        int sig_caught;
        sigwait( &signal_mask, &sig_caught );

        if ( sig_caught == SIGINT ) {
          EXAM_MESSAGE_ASYNC( "catch INT signal" );
          std::tr2::unique_lock<std::tr2::mutex> lk(Tester::lock);
          EXAM_CHECK_ASYNC_F( Tester::cnd.timed_wait( lk, std::tr2::milliseconds( 2000 ), Tester::n_cnt ), flag );
          // {
          // std::tr2::lock_guard<std::tr2::mutex_ip> lk2(m);
          // j = 1;
          c.notify_one();
          // }
          srv.close();
          srv.wait();
        } else {
          EXAM_ERROR_ASYNC_F( "catch of INT signal expected", flag );
        }

      }
      catch ( ... ) {
        EXAM_ERROR_ASYNC_F( "unexpected exception", flag );
      }

      exit(flag);
    }
    catch ( std::tr2::fork_in_parent& child ) {
      b.wait();

      stem::NetTransportMgr mgr;

      stem::addr_type zero = mgr.open( "localhost", port );

      Tester t1;

      Event ev( EV_TEST );
      ev.dest( zero );

      for ( int i = 0; i < N; ++i ) {
        t1.Send( ev );
      }


      kill( child.pid(), SIGINT );

      // {
      // std::tr2::unique_lock<std::tr2::mutex_ip> lk(m);
      EXAM_CHECK( c.timed_wait( std::tr2::milliseconds( 2000 ) ) );
      // }

      // this_thread::sleep( milliseconds( 500 ) );

      mgr.close();

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }
    }

    shm.deallocate( &b );
    // shmcnd.deallocate( &c );
    // shmm.deallocate( &m );
    shme.deallocate( &c );

    seg.deallocate();
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  Tester::visits = 0;

  return EXAM_RESULT;
}

int EXAM_IMPL(stem_perf::net_loopback_inv)
{
  try {
    xmt::shm_alloc<0> seg;
    seg.allocate( 70000, 4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    xmt::allocator_shm<std::tr2::barrier_ip,0> shm;
    xmt::allocator_shm<std::tr2::condition_event_ip,0> shme;

    std::tr2::barrier_ip& b = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip();
    std::tr2::condition_event_ip& c = *new ( shme.allocate( 1 ) ) std::tr2::condition_event_ip();

    try {
      std::tr2::this_thread::fork();

      int flag = 0;

      try {
        std::tr2::this_thread::block_signal( SIGINT );

        sigset_t signal_mask;

        sigemptyset( &signal_mask );
        sigaddset( &signal_mask, SIGINT );

        b.wait();

        stem::NetTransportMgr mgr;

        stem::addr_type zero = mgr.open( "localhost", port );

        Tester t1;

        Event ev( EV_TEST );
        ev.dest( zero );

        for ( int i = 0; i < N; ++i ) {
          t1.Send( ev );
        }

        int sig_caught;
        sigwait( &signal_mask, &sig_caught );

        if ( sig_caught == SIGINT ) {
          EXAM_MESSAGE_ASYNC( "catch INT signal" );
          
          mgr.close();
          c.notify_one();

        } else {
          EXAM_ERROR_ASYNC_F( "catch of INT signal expected", flag );
        }

      }
      catch ( ... ) {
        EXAM_ERROR_ASYNC_F( "unexpected exception", flag );
      }

      exit(flag);
    }
    catch ( std::tr2::fork_in_parent& child ) {
      std::connect_processor<NetTransport> srv( port );
      
      EXAM_CHECK( srv.good() );
      EXAM_CHECK( srv.is_open() );

      Tester t2;
      t2.set_default();

      b.wait();

      {
        std::tr2::unique_lock<std::tr2::mutex> lk(Tester::lock);
        EXAM_CHECK( Tester::cnd.timed_wait( lk, std::tr2::milliseconds( 2000 ), Tester::n_cnt ) );
      }

      kill( child.pid(), SIGINT );

      EXAM_CHECK( c.timed_wait( std::tr2::milliseconds( 500 ) ) );

      srv.close();
      srv.wait();

      // this_thread::sleep( milliseconds( 500 ) );

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }
    }

    shm.deallocate( &b );
    shme.deallocate( &c );

    seg.deallocate();
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  Tester::visits = 0;

  return EXAM_RESULT;
}

int EXAM_IMPL(stem_perf::net_loopback_inv2)
{
  try {
    xmt::shm_alloc<0> seg;
    seg.allocate( 70000, 4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    xmt::allocator_shm<std::tr2::barrier_ip,0> shm;
    xmt::allocator_shm<std::tr2::condition_event_ip,0> shme;

    std::tr2::barrier_ip& b = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip();
    std::tr2::condition_event_ip& c = *new ( shme.allocate( 1 ) ) std::tr2::condition_event_ip();

    try {
      std::tr2::this_thread::fork();

      int flag = 0;

      try {
        std::tr2::this_thread::block_signal( SIGINT );

        std::connect_processor<NetTransport> srv( port );

        EXAM_CHECK_ASYNC_F( srv.good(), flag );
        EXAM_CHECK_ASYNC_F( srv.is_open(), flag );

        sigset_t signal_mask;

        sigemptyset( &signal_mask );
        sigaddset( &signal_mask, SIGINT );

        Tester t2;
        t2.set_default();

        b.wait();

        if ( t2.wait_peer() ) {
          t2.send();
        } else {
          EXAM_ERROR_ASYNC_F( "peer address not set", flag );
        }

        int sig_caught;
        sigwait( &signal_mask, &sig_caught );

        if ( sig_caught == SIGINT ) {
          EXAM_MESSAGE_ASYNC( "catch INT signal" );
          c.notify_one();
          srv.close();
          srv.wait();
        } else {
          EXAM_ERROR_ASYNC_F( "catch of INT signal expected", flag );
        }

      }
      catch ( ... ) {
        EXAM_ERROR_ASYNC_F( "unexpected exception", flag );
      }

      exit(flag);
    }
    catch ( std::tr2::fork_in_parent& child ) {
      b.wait();

      stem::NetTransportMgr mgr;

      stem::addr_type zero = mgr.open( "localhost", port );

      Tester t1;

      Event ev( EV_SETPEER );
      ev.dest( zero );

      t1.Send( ev );

      {
        std::tr2::unique_lock<std::tr2::mutex> lk(Tester::lock);
        EXAM_CHECK( Tester::cnd.timed_wait( lk, std::tr2::milliseconds( 2000 ), Tester::n_cnt ) );
      }

      kill( child.pid(), SIGINT );

      EXAM_CHECK( c.timed_wait( std::tr2::milliseconds( 500 ) ) );

      // this_thread::sleep( milliseconds( 500 ) );

      mgr.close();

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }
    }

    shm.deallocate( &b );
    shme.deallocate( &c );

    seg.deallocate();
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  Tester::visits = 0;

  return EXAM_RESULT;
}

int EXAM_IMPL(stem_perf::parallel)
{
  stem::addr_type a1 = xmt::uid();
  stem::addr_type a2 = xmt::uid();

  Busy b1( a1 );
  Busy b2( a2 );

  Tester t;

  Event ev1( EV_TEST );
  ev1.dest( a1 );

  Event ev2( EV_TEST );
  ev2.dest( a2 );

  for ( int i = 0; i < b1.cnt_lim; ++i ) {
    t.Send( ev1 );
    t.Send( ev2 );
  }

  EXAM_CHECK( b1.wait() );
  EXAM_CHECK( b2.wait() );

  return EXAM_RESULT;
}
