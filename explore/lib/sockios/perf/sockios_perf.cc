// -*- C++ -*- Time-stamp: <09/07/29 15:18:20 ptr>

/*
 *
 * Copyright (c) 2007, 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "sockios_perf.h"
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

using namespace std;
using namespace std::tr2;

// std::tr2::mutex server_conn::lock;
// std::tr2::condition_variable server_conn::cnd;

int EXAM_IMPL(sockios_perf_conn::connect)
{
  int flag = 0;

  try {
    xmt::shm_alloc<0> seg;
    seg.allocate( 70000, 4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    xmt::allocator_shm<std::tr2::barrier_ip,0> shm;

    std::tr2::barrier_ip& b = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip();

    try {
      std::tr2::this_thread::fork();

      try {
        std::tr2::this_thread::block_signal( SIGINT );

        std::connect_processor<server_conn> srv( 6480 );

        EXAM_CHECK_ASYNC_F( srv.good(), flag );
        EXAM_CHECK_ASYNC_F( srv.is_open(), flag );

        sigset_t signal_mask;

        sigemptyset( &signal_mask );
        sigaddset( &signal_mask, SIGINT );

        b.wait();

        int sig_caught;
        sigwait( &signal_mask, &sig_caught );

        if ( sig_caught == SIGINT ) {
          EXAM_MESSAGE_ASYNC( "catch INT signal" );
          // std::tr2::unique_lock<std::tr2::mutex> lk(server_conn::lock);
          // server_conn::cnd.timed_wait( lk, std::tr2::milliseconds( 500 ), server_conn::n_cnt );
          srv.close();
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

      for ( int i = 0; i < 1000; ++i ) {
        std::sockstream s( "localhost", 6480 );
        // if ( ND ) {
        //   s.rdbuf()->setoptions( std::sock_base::so_tcp_nodelay );
        // }
        EXAM_CHECK( s.is_open() );
      }

      kill( child.pid(), SIGINT );

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }
    }

    shm.deallocate( &b );

    seg.deallocate();
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(sockios_perf_conn::connect_basic)
{
  int flag = 0;

  try {
    xmt::shm_alloc<0> seg;
    seg.allocate( 70000, 4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    xmt::allocator_shm<std::tr2::barrier_ip,0> shm;

    std::tr2::barrier_ip& b = *new ( shm.allocate( 1 ) ) std::tr2::barrier_ip();

    try {
      std::tr2::this_thread::fork();

      try {
#if 0
        std::tr2::this_thread::unblock_signal( SIGINT );

        int fd = socket( PF_INET, sock_base::sock_stream, 0 );

        EXAM_CHECK_ASYNC_F( fd >= 0, flag );

        int f = 1;
        setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &f, sizeof(int) );

        union sockaddr_t {
            sockaddr_in inet;
            sockaddr_un unx;
            sockaddr    any;
        } _address;

        _address.inet.sin_family = AF_INET;
        _address.inet.sin_port = htons( uint16_t(6480) );
        _address.inet.sin_addr.s_addr = htonl( 0x7f000001UL );

        // EXAM_CHECK_ASYNC_F( ::bind( fd, &_address.any, sizeof(_address) ) == 0, flag );
        int res = ::bind( fd, &_address.any, sizeof(_address) );
        if ( res < 0 ) {
          // EXAM_ERROR_ASYNC_F( "bind", flag );
          cerr << std::system_error( errno, std::get_posix_category() ).what() << endl;
        }

        EXAM_CHECK_ASYNC_F( ::listen( fd, SOMAXCONN ) == 0, flag);

        socklen_t sz = sizeof(_address);

        b.wait();

        for ( ; ; ) {
          int fd_conn = accept( fd, &_address.any, &sz );
          EXAM_CHECK_ASYNC_F( fd_conn >= 0, flag );
          close(fd_conn);
        }
#else
        std::tr2::this_thread::block_signal( SIGINT );

        std::connect_processor<server_conn> srv( 6480 );

        EXAM_CHECK_ASYNC_F( srv.good(), flag );
        EXAM_CHECK_ASYNC_F( srv.is_open(), flag );

        sigset_t signal_mask;

        sigemptyset( &signal_mask );
        sigaddset( &signal_mask, SIGINT );

        b.wait();

        int sig_caught;
        sigwait( &signal_mask, &sig_caught );

        if ( sig_caught == SIGINT ) {
          EXAM_MESSAGE_ASYNC( "catch INT signal" );
          // std::tr2::unique_lock<std::tr2::mutex> lk(server_conn::lock);
          // server_conn::cnd.timed_wait( lk, std::tr2::milliseconds( 500 ), server_conn::n_cnt );
          srv.close();
        } else {
          EXAM_ERROR_ASYNC_F( "catch of INT signal expected", flag );
        }
#endif
      }
      catch ( ... ) {
        EXAM_ERROR_ASYNC_F( "unexpected exception", flag );
      }

      exit(flag);
    }
    catch ( std::tr2::fork_in_parent& child ) {
      b.wait();

#if 0
      for ( int i = 0; i < 1000; ++i ) {
        std::sockstream s( "localhost", 6480 );
        // if ( ND ) {
        //   s.rdbuf()->setoptions( std::sock_base::so_tcp_nodelay );
        // }
        // EXAM_CHECK( s.is_open() );
      }
#else
      union sockaddr_t {
          sockaddr_in inet;
          sockaddr_un unx;
          sockaddr    any;
      } _address;

      _address.inet.sin_family = AF_INET;
      _address.inet.sin_port = htons( uint16_t(6480) );
      _address.inet.sin_addr.s_addr = htonl( 0x7f000001UL );

      for ( int i = 0; i < 1000; ++i ) {
        int fd = socket( PF_INET, sock_base::sock_stream, 0 );
        EXAM_CHECK( fd >= 0 );

        EXAM_CHECK( ::connect( fd, &_address.any, sizeof(_address) ) == 0 );
        close( fd );
      }
#endif

      kill( child.pid(), SIGINT );

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        // EXAM_ERROR( "child interrupted" );
      }
    }

    shm.deallocate( &b );

    seg.deallocate();
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

sockios_perf_SrvR::sockios_perf_SrvR()
{
}

sockios_perf_SrvR::~sockios_perf_SrvR()
{
}

sockios_perf_SrvW::sockios_perf_SrvW()
{
}

sockios_perf_SrvW::~sockios_perf_SrvW()
{
}

sockios_perf_SrvRW::sockios_perf_SrvRW()
{
}

sockios_perf_SrvRW::~sockios_perf_SrvRW()
{
}

