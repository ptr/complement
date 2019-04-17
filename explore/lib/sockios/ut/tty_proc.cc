/* -*- C++ -*- */

/*
 *
 * Copyright (c) 2019
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "tty_proc.h"

#include <exam/suite.h>

#include <sockios/sockstream>
#include <sockios/sockmgr.h>
#include <sockios/socksrv.h>

#include <mt/mutex>
#include <mt/condition_variable>
#include <mt/shm.h>
#include <sys/wait.h>

using namespace std;
using namespace std::tr2;

class simple_tty_mgr :
    public sock_basic_processor
{
  public:
    simple_tty_mgr() :
        sock_basic_processor()
      { fill( buf, buf + 1024, 0 ); }

    simple_tty_mgr( const char* path, sock_base::stype t = sock_base::tty ) :
        sock_basic_processor( path, t )
      { fill( buf, buf + 1024, 0 ); }

    ~simple_tty_mgr()
      { }

  protected:
    virtual sock_basic_processor::sockbuf_t* operator ()( sock_base::socket_type fd, const sockaddr& )
      {
        lock_guard<mutex> lk(lock);
        ++n_cnt;
        /* size_t n = */ ::read( fd, buf, 1024 );
        cnd.notify_one();
        return 0;
      }
    virtual void operator ()( sock_base::socket_type, const adopt_close_t& )
      { lock_guard<mutex> lk(lock); ++c_cnt; cnd.notify_one(); }
    virtual void operator ()( sock_base::socket_type )
      { lock_guard<mutex> lk(lock); ++d_cnt; cnd.notify_one(); }

  public:
    static mutex lock;
    static int n_cnt;
    static int c_cnt;
    static int d_cnt;
    static condition_variable cnd;
    static char buf[];

    static bool n_cnt_check()
      { return n_cnt == 1; }
    static bool c_cnt_check()
      { return c_cnt == 1; }
    static bool d_cnt_check()
      { return d_cnt == 1; }
};

mutex simple_tty_mgr::lock;
int simple_tty_mgr::n_cnt = 0;
int simple_tty_mgr::c_cnt = 0;
int simple_tty_mgr::d_cnt = 0;
condition_variable simple_tty_mgr::cnd;
char simple_tty_mgr::buf[1024];

int EXAM_IMPL(tty_processor_test::tty_sockbuf)
{
  try {
    xmt::shm_alloc<0> seg;
    seg.allocate( 70000, 4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    xmt::allocator_shm<barrier_ip,0> shm;

    barrier_ip& b = *new ( shm.allocate( 1 ) ) barrier_ip();

    int ptm = getpt() /* ::open("/dev/ptmx", O_RDWR | O_NOCTTY) */;
    EXAM_CHECK(ptm >= 0);

    char buf[128];

    int ret = grantpt(ptm);
    EXAM_CHECK(ret == 0);
    ret = unlockpt(ptm);
    EXAM_CHECK(ret == 0);

    ret = ptsname_r(ptm, buf, 128);
    EXAM_CHECK(ret == 0);

    try {
      this_thread::fork();
      int ret = 0;

      close(ptm);

      if (ret == 0) {
        sockbuf s;

        // open slave PTTY
        EXAM_CHECK_ASYNC_F(s.open(buf, sock_base::tty) != 0, ret);

        b.wait(true); // <--- align here
        if (s.is_open()) {
          // this is RAW TTY:
          s.setoptions(0, IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON,
                       0, OPOST,
                       CS8, CSIZE | PARENB,
                       0, ECHO | ECHONL | ICANON | ISIG | IEXTEN,
                       TCSADRAIN);

          /* Reader should be present on slave, otherwise master will receive HUP
             on epoll.
           */
          char c = s.sgetc();
          EXAM_CHECK_ASYNC_F(c == '.', ret);

          b.wait(true);
        }
      } else {
        b.wait(true); // <--- align here (instead of wait in positive branch above)
      }
      b.wait(); // <--- align here

      exit(ret);
    }
    catch ( std::tr2::fork_in_parent& child ) {
      sockbuf s;

      // open master PTTY
      EXAM_CHECK(s.open(ptm, sock_base::tty) != 0);

      b.wait(true); // <--- align here

      if (s.is_open()) {
        // this is RAW TTY
        s.setoptions(0, IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON,
                     0, OPOST,
                     CS8, CSIZE | PARENB,
                     0, ECHO | ECHONL | ICANON | ISIG | IEXTEN,
                     TCSADRAIN
          );
        s.sputc('.');
        s.pubsync();
        b.wait(true);
      }

      b.wait(); // <--- align here

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }
    }

    if (ptm >= 0) {
      ::close(ptm);
    }

    shm.deallocate( &b );
    seg.deallocate();
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(tty_processor_test::tty_sockstream)
{
  try {
    xmt::shm_alloc<0> seg;
    seg.allocate( 70000, 4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    xmt::allocator_shm<barrier_ip,0> shm;

    barrier_ip& b = *new ( shm.allocate( 1 ) ) barrier_ip();

    int ptm = getpt() /* ::open("/dev/ptmx", O_RDWR | O_NOCTTY) */;
    EXAM_CHECK(ptm >= 0);

    char buf[128];

    int ret = grantpt(ptm);
    EXAM_CHECK(ret == 0);
    ret = unlockpt(ptm);
    EXAM_CHECK(ret == 0);

    ret = ptsname_r(ptm, buf, 128);
    EXAM_CHECK(ret == 0);

    try {
      this_thread::fork();
      int ret = 0;

      close(ptm);

      if (ret == 0) {
        sockstream s;

        // open slave PTTY
        s.open(buf, sock_base::tty);
        EXAM_CHECK_ASYNC_F(s.is_open() && s.good(), ret);

        b.wait(true); // <--- align here
        if (s.is_open()) {
          // this is RAW TTY:
          s.rdbuf()->setoptions(0, IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON,
                                0, OPOST,
                                CS8, CSIZE | PARENB,
                                0, ECHO | ECHONL | ICANON | ISIG | IEXTEN,
                                TCSADRAIN);

          /* Reader should be present on slave, otherwise master will receive HUP
             on epoll.
           */
          char c;
          s.read(&c, 1);
          EXAM_CHECK_ASYNC_F(c == '.', ret);

          b.wait(true);
        }
      } else {
        b.wait(true); // <--- align here (instead of wait in positive branch above)
      }
      b.wait(); // <--- align here

      exit(ret);
    }
    catch ( std::tr2::fork_in_parent& child ) {
      sockstream s;

      // open master PTTY
      s.open(ptm, sock_base::tty);
      EXAM_CHECK(s.is_open() && s.good());

      b.wait(true); // <--- align here

      if (s.is_open()) {
        // this is RAW TTY
        s.rdbuf()->setoptions(0, IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON,
                              0, OPOST,
                              CS8, CSIZE | PARENB,
                              0, ECHO | ECHONL | ICANON | ISIG | IEXTEN,
                              TCSADRAIN
          );
        s << '.';
        s.flush();
        b.wait(true);
      }

      b.wait(); // <--- align here

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }
    }

    if (ptm >= 0) {
      ::close(ptm);
    }

    shm.deallocate( &b );
    seg.deallocate();
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

int EXAM_IMPL(tty_processor_test::tty_processor)
{
  try {
    xmt::shm_alloc<0> seg;
    seg.allocate( 70000, 4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    xmt::allocator_shm<barrier_ip,0> shm;

    barrier_ip& b = *new ( shm.allocate( 1 ) ) barrier_ip();

    int ptm = getpt() /* ::open("/dev/ptmx", O_RDWR | O_NOCTTY) */;
    EXAM_CHECK(ptm >= 0);

    char buf[128];

    int ret = grantpt(ptm);
    EXAM_CHECK(ret == 0);
    ret = unlockpt(ptm);
    EXAM_CHECK(ret == 0);

    ret = ptsname_r(ptm, buf, 128);
    EXAM_CHECK(ret == 0);

    try {
      this_thread::fork();
      int ret = 0;

      close(ptm);

      if (ret == 0) {
        simple_tty_mgr srv(buf, sock_base::tty); // listen on slave PTTY

        EXAM_CHECK_ASYNC_F( srv.is_open(), ret );
        EXAM_CHECK_ASYNC_F( srv.good(), ret );

        b.wait(true); // <--- align here
        if (srv.is_open()) {
          {
            unique_lock<mutex> lk( simple_tty_mgr::lock );
            EXAM_CHECK_ASYNC_F(simple_tty_mgr::cnd.timed_wait( lk, milliseconds( 500 ), simple_tty_mgr::n_cnt_check), ret);
            EXAM_CHECK_ASYNC_F(simple_tty_mgr::buf[0] == '.', ret);
          }

          b.wait(true);
        }

        srv.close();

        EXAM_CHECK_ASYNC_F(!srv.is_open(), ret);
      } else {
        b.wait(true); // <--- align here (instead of wait in positive branch above)
      }
      b.wait(); // <--- align here

      exit(ret);
    }
    catch ( std::tr2::fork_in_parent& child ) {
      sockstream s(ptm, sock_base::tty); // open master PTTY

      EXAM_CHECK(s.is_open() && s.good());

      b.wait(true); // <--- align here

      if (s.is_open()) {
        // this is RAW TTY
        s.rdbuf()->setoptions(0, IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON,
                              0, OPOST,
                              CS8, CSIZE | PARENB,
                              0, ECHO | ECHONL | ICANON | ISIG | IEXTEN,
                              TCSADRAIN
          );
        s << '.';
        s.flush();
        b.wait(true);
      }

      b.wait(); // <--- align here

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }
    }

    if (ptm >= 0) {
      ::close(ptm);
    }

    shm.deallocate( &b );
    seg.deallocate();
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}

class packet_s
{
  public:
    packet_s() = delete;

    packet_s(std::sock_basic_processor& srv) :
        _srv(srv)
      { }
    void operator ()(sockstream&);

  private:
    std::sock_basic_processor& _srv;

  public:
    static mutex lock;
    static char rc;
};

mutex packet_s::lock;
char packet_s::rc;

void packet_s::operator ()(sockstream& s)
{
  unique_lock<mutex> lk(lock);

  s.read(&rc, 1);

  _srv.close();
}

int EXAM_IMPL(tty_processor_test::tty_packet_processor)
{
  try {
    xmt::shm_alloc<0> seg;
    seg.allocate( 70000, 4096, xmt::shm_base::create | xmt::shm_base::exclusive, 0660 );

    xmt::allocator_shm<barrier_ip,0> shm;

    barrier_ip& b = *new ( shm.allocate( 1 ) ) barrier_ip();

    int ptm = getpt() /* ::open("/dev/ptmx", O_RDWR | O_NOCTTY) */;
    EXAM_CHECK(ptm >= 0);

    char buf[128];

    int ret = grantpt(ptm);
    EXAM_CHECK(ret == 0);
    ret = unlockpt(ptm);
    EXAM_CHECK(ret == 0);

    ret = ptsname_r(ptm, buf, 128);
    EXAM_CHECK(ret == 0);

    try {
      this_thread::fork();
      int ret = 0;

      close(ptm);

      if (ret == 0) {
        {
          unique_lock<mutex> lk(packet_s::lock);
          packet_s::rc = '-';
        }
        packet_processor<packet_s> srv(buf, sock_base::tty); // listen on slave PTTY

        EXAM_CHECK_ASYNC_F( srv.is_open(), ret );
        EXAM_CHECK_ASYNC_F( srv.good(), ret );

        b.wait(true); // <--- align here
        if (srv.is_open()) {
          b.wait(true);

          srv.wait();
        }

        EXAM_CHECK_ASYNC_F(!srv.is_open(), ret);
        {
          unique_lock<mutex> lk(packet_s::lock);
          EXAM_CHECK_ASYNC_F(packet_s::rc == '.', ret);
        }
      } else {
        b.wait(true); // <--- align here (instead of wait in positive branch above)
      }
      b.wait(); // <--- align here

      exit(ret);
    }
    catch ( std::tr2::fork_in_parent& child ) {
      sockstream s(ptm, sock_base::tty); // open master PTTY

      EXAM_CHECK(s.is_open() && s.good());

      b.wait(true); // <--- align here

      if (s.is_open()) {
        // this is RAW TTY
        s.rdbuf()->setoptions(0, IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON,
                              0, OPOST,
                              CS8, CSIZE | PARENB,
                              0, ECHO | ECHONL | ICANON | ISIG | IEXTEN,
                              TCSADRAIN
          );
        s << '.';
        s.flush();
        b.wait(true);
      }

      b.wait(); // <--- align here

      int stat = -1;
      EXAM_CHECK( waitpid( child.pid(), &stat, 0 ) == child.pid() );
      if ( WIFEXITED(stat) ) {
        EXAM_CHECK( WEXITSTATUS(stat) == 0 );
      } else {
        EXAM_ERROR( "child interrupted" );
      }
    }

    if (ptm >= 0) {
      ::close(ptm);
    }

    shm.deallocate( &b );
    seg.deallocate();
  }
  catch ( xmt::shm_bad_alloc& err ) {
    EXAM_ERROR( err.what() );
  }

  return EXAM_RESULT;
}
