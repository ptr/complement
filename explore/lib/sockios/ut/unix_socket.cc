// -*- C++ -*- Time-stamp: <09/06/17 19:22:59 ptr>

/*
 *
 * Copyright (c) 2009
 * Petr Ovtchenkov
 *
 * Licensed under the Academic Free License version 3.0
 *
 */

#include "unix_socket.h"

#include <exam/suite.h>

#include <sockios/sockstream>
#include <sockios/sockmgr.h>

#include <mt/mutex>
#include <mt/condition_variable>

#include <sockios/syslog.h>
#include <locale>

using namespace std;
using namespace std::tr2;

class simple_us_mgr :
    public sock_basic_processor
{
  public:
    simple_us_mgr() :
        sock_basic_processor()
      { }

    simple_us_mgr( const char* path, sock_base::stype t = sock_base::sock_dgram ) :
        sock_basic_processor( path, t )
      { }

    ~simple_us_mgr()
      { /* cerr << "In destructor\n"; */ }

  protected:
    virtual sock_basic_processor::sockbuf_t* operator ()( sock_base::socket_type, const sockaddr& )
      { lock_guard<mutex> lk(lock); ++n_cnt; cnd.notify_one(); return 0; }
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

    static bool n_cnt_check()
      { return n_cnt == 1; }
    static bool c_cnt_check()
      { return c_cnt == 1; }
    static bool d_cnt_check()
      { return d_cnt == 1; }
};

mutex simple_us_mgr::lock;
int simple_us_mgr::n_cnt = 0;
int simple_us_mgr::c_cnt = 0;
int simple_us_mgr::d_cnt = 0;
condition_variable simple_us_mgr::cnd;

int EXAM_IMPL(unix_sockios_test::core_test)
{
  // throw exam::skip_exception();

  const char* f = "/tmp/sock_test";
  // const char* f = "/dev/log";
  {
    simple_us_mgr s( f );

    EXAM_CHECK( s.is_open() );
    EXAM_CHECK( s.good() );

    {
      sockstream str( f );

      EXAM_CHECK( str.is_open() );
      EXAM_CHECK( str.good() );

      {
        unique_lock<mutex> lk( simple_us_mgr::lock );

        // EXAM_CHECK( simple_us_mgr::cnd.timed_wait( lk, milliseconds( 500 ), simple_us_mgr::n_cnt_check ) );
      }
      {
        lock_guard<mutex> lk(simple_us_mgr::lock);
        EXAM_CHECK( simple_us_mgr::d_cnt == 0 );
      }
      {
        lock_guard<mutex> lk(simple_us_mgr::lock);
        EXAM_CHECK( simple_us_mgr::c_cnt == 0 );
      }

      str.close();
    }

    unique_lock<mutex> lk( simple_us_mgr::lock );

    // EXAM_CHECK( simple_us_mgr::cnd.timed_wait( lk, milliseconds( 500 ), simple_us_mgr::c_cnt_check ) );
    EXAM_CHECK( simple_us_mgr::d_cnt == 0 );

    s.close();
  }

  unlink( f );

#if 0
  {
    misc::open_syslog();
    int i = 20;
    misc::use_syslog<LOG_INFO,LOG_USER>() << "hello " << i << ", ok" << endl;
    misc::close_syslog();
  }
#endif

  return EXAM_RESULT;
}
