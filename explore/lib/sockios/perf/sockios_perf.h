// -*- C++ -*- Time-stamp: <09/07/17 17:14:28 ptr>

/*
 *
 * Copyright (c) 2007, 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#ifndef __sockios_perf_h
#define __sockios_perf_h

#include <exam/suite.h>
#include <string>

#include <sockios/sockstream>
#include <sockios/sockmgr.h>

#include <mt/shm.h>
#include <sys/wait.h>
#include <signal.h>
#include <algorithm>

#include <mt/mutex>
#include <mt/condition_variable>
#include <mt/date_time>

class sockios_perf_SrvR
{
  public:
    sockios_perf_SrvR();
    ~sockios_perf_SrvR();

    template <int N, int S, int P, int ND>
    int EXAM_DECL(rx);
};

template <int N, int S, int P, int ND>
int block_write();

template <int N, int S, int P, int ND>
int EXAM_IMPL(sockios_perf_SrvR::rx)
{
  return block_write<N,S,P,ND>();
}

template <int N, int S, int ND>
class SrvR
{
  public:
    SrvR( std::sockstream& s )
      {
        if ( ND ) {
          s.rdbuf()->setoptions( std::sock_base::so_tcp_nodelay );
        }
        fill( buf, buf + S, 'b' );
      }

    ~SrvR()
      { }

    void connect( std::sockstream& s )
      {
        EXAM_CHECK_ASYNC( s.good() );

        s.read( buf, S );

        EXAM_CHECK_ASYNC( !s.fail() );

        std::tr2::lock_guard<std::tr2::mutex> lk(lock);
        if ( ++visits == N ) {
          cnd.notify_one();
        }
      }

    static bool n_cnt()
      { return visits == N; }

  private:
    char buf[S];

  public:
    static std::tr2::mutex lock;
    static std::tr2::condition_variable cnd;
    static int visits;
};

template <int N, int S, int ND>
std::tr2::mutex SrvR<N,S,ND>::lock;

template <int N, int S, int ND>
std::tr2::condition_variable SrvR<N,S,ND>::cnd;

template <int N, int S, int ND>
int SrvR<N,S,ND>::visits = 0;

template <int N, int S, int P, int ND>
int block_write()
{
  typedef SrvR<N,S,ND> processor;

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

        std::connect_processor<processor> srv( P );

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
          std::tr2::unique_lock<std::tr2::mutex> lk(processor::lock);
          processor::cnd.timed_wait( lk, std::tr2::milliseconds( 500 ), processor::n_cnt );
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

      std::sockstream s( "localhost", P );
      if ( ND ) {
        s.rdbuf()->setoptions( std::sock_base::so_tcp_nodelay );
      }

      char buf[S];
      std::fill( buf, buf + S, 'a' );
      for ( int i = 0; i < N; ++i ) {
        s.write( buf, S ).flush();
        EXAM_CHECK_ASYNC_F( s.good(), flag );
        if ( s.fail() ) {
          break; // no sense to continue
        }
      }

      // this_thread::sleep( milliseconds( 500 ) );

      kill( child.pid(), SIGINT );

      int stat = -1;
      EXAM_CHECK_ASYNC_F( waitpid( child.pid(), &stat, 0 ) == child.pid(), flag );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK_ASYNC_F( WEXITSTATUS(stat) == 0, flag );
      } else {
        EXAM_ERROR_ASYNC_F( "child interrupted", flag );
      }
    }

    processor::visits = 0;

    shm.deallocate( &b );

    seg.deallocate();
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR_ASYNC_F( err.what(), flag );
  }

  return flag;
}

template <int N, int S, int ND>
class SrvW
{
  public:
    SrvW( std::sockstream& s )
      {
        if ( ND ) {
          s.rdbuf()->setoptions( std::sock_base::so_tcp_nodelay );
        }

        fill( buf, buf + S, 'b' );
        for ( int i = 0; i < N; ++i ) {
          s.write( buf, S ).flush();
          EXAM_CHECK_ASYNC( !s.fail() );
        }
        std::tr2::lock_guard<std::tr2::mutex> lk(lock);
        if ( ++visits == N ) {
          cnd.notify_one();
        }
      }

    ~SrvW()
      { }

    void connect( std::sockstream& s )
      {
      }

    static bool n_cnt()
      { return visits == N; }

  private:
    char buf[S];

  public:
    static std::tr2::mutex lock;
    static std::tr2::condition_variable cnd;
    static int visits;
};

template <int N, int S, int ND>
std::tr2::mutex SrvW<N,S,ND>::lock;

template <int N, int S, int ND>
std::tr2::condition_variable SrvW<N,S,ND>::cnd;

template <int N, int S, int ND>
int SrvW<N,S,ND>::visits = 0;

template <int N, int S, int P, int ND>
int block_read()
{
  typedef SrvW<N,S,ND> processor;

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

        std::connect_processor<processor> srv( P );

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
          std::tr2::unique_lock<std::tr2::mutex> lk(processor::lock);
          processor::cnd.timed_wait( lk, std::tr2::milliseconds( 500 ), processor::n_cnt );
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

      std::sockstream s( "localhost", P );
      if ( ND ) {
        s.rdbuf()->setoptions( std::sock_base::so_tcp_nodelay );
      }

      char buf[S];
      std::fill( buf, buf + S, 'a' );
      for ( int i = 0; i < N; ++i ) {
        s.read( buf, S );
        EXAM_CHECK_ASYNC_F( s.good(), flag );
      }

      // this_thread::sleep( milliseconds( 500 ) );

      kill( child.pid(), SIGINT );

      int stat = -1;
      EXAM_CHECK_ASYNC_F( waitpid( child.pid(), &stat, 0 ) == child.pid(), flag );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK_ASYNC_F( WEXITSTATUS(stat) == 0, flag );
      } else {
        EXAM_ERROR_ASYNC_F( "child interrupted", flag );
      }
    }

    processor::visits = 0;

    shm.deallocate( &b );

    seg.deallocate();
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR_ASYNC_F( err.what(), flag );
  }

  return flag;
}

class sockios_perf_SrvW
{
  public:
    sockios_perf_SrvW();
    ~sockios_perf_SrvW();

    template <int N, int S, int P, int ND>
    int EXAM_DECL(rx);
};

template <int N, int S, int P, int ND>
int block_read();

template <int N, int S, int P, int ND>
int EXAM_IMPL(sockios_perf_SrvW::rx)
{
  return block_read<N,S,P,ND>();
}

class sockios_perf_SrvRW
{
  public:
    sockios_perf_SrvRW();
    ~sockios_perf_SrvRW();

    template <int N, int S, int P, int ND>
    int EXAM_DECL(rx);
};

template <int N, int S, int P, int ND>
int block_read_write();

template <int N, int S, int P, int ND>
int EXAM_IMPL(sockios_perf_SrvRW::rx)
{
  return block_read_write<N/2,S,P,ND>();
}

template <int N, int S, int ND>
class SrvRW
{
  public:
    SrvRW( std::sockstream& s )
      {
        if ( ND ) {
          s.rdbuf()->setoptions( std::sock_base::so_tcp_nodelay );
        }

        fill( buf, buf + S, 'b' );
      }

    ~SrvRW()
      { }

    void connect( std::sockstream& s )
      {
        EXAM_CHECK_ASYNC( s.good() );

        s.read( buf, S );
        s.write( buf, S ).flush();

        EXAM_CHECK_ASYNC( !s.fail() );

        std::tr2::lock_guard<std::tr2::mutex> lk(lock);
        if ( ++visits == N ) {
          cnd.notify_one();
        }
      }

    static bool n_cnt()
      { return visits == N; }

  private:
    char buf[S];

  public:
    static std::tr2::mutex lock;
    static std::tr2::condition_variable cnd;
    static int visits;
};

template <int N, int S, int ND>
std::tr2::mutex SrvRW<N,S,ND>::lock;

template <int N, int S, int ND>
std::tr2::condition_variable SrvRW<N,S,ND>::cnd;

template <int N, int S, int ND>
int SrvRW<N,S,ND>::visits = 0;

template <int N, int S, int P, int ND>
int block_read_write()
{
  typedef SrvRW<N,S,ND> processor;

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

        std::connect_processor<processor> srv( P );

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
          std::tr2::unique_lock<std::tr2::mutex> lk(processor::lock);
          processor::cnd.timed_wait( lk, std::tr2::milliseconds( 500 ), processor::n_cnt );
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

      std::sockstream s( "localhost", P );

      if ( ND ) {
        s.rdbuf()->setoptions( std::sock_base::so_tcp_nodelay );
      }

      char buf[S];
      std::fill( buf, buf + S, 'a' );
      for ( int i = 0; i < N; ++i ) {
        s.write( buf, S ).flush();
        s.read( buf, S );
        EXAM_CHECK_ASYNC_F( s.good(), flag );
      }

      // this_thread::sleep( milliseconds( 500 ) );

      kill( child.pid(), SIGINT );

      int stat = -1;
      EXAM_CHECK_ASYNC_F( waitpid( child.pid(), &stat, 0 ) == child.pid(), flag );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK_ASYNC_F( WEXITSTATUS(stat) == 0, flag );
      } else {
        EXAM_ERROR_ASYNC_F( "child interrupted", flag );
      }
    }

    processor::visits = 0;

    shm.deallocate( &b );

    seg.deallocate();
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR_ASYNC_F( err.what(), flag );
  }

  return flag;
}

#endif // __sockios_perf_h
